import logging
from datetime import datetime, timedelta
from typing import List, Mapping, Optional, Sequence, Set, Tuple

import pytz
from django.db import transaction
from django.http import Http404, HttpResponse, StreamingHttpResponse
from rest_framework.request import Request
from rest_framework.response import Response
from symbolic import SymbolicError, normalize_debug_id

from sentry import options, ratelimits
from sentry.api.base import region_silo_endpoint
from sentry.api.bases.project import ProjectEndpoint, ProjectReleasePermission
from sentry.api.endpoints.debug_files import has_download_permission
from sentry.api.serializers import serialize
from sentry.auth.system import is_system_auth
from sentry.lang.native.sources import get_internal_artifact_lookup_source_url
from sentry.models import (
    ArtifactBundle,
    DebugIdArtifactBundle,
    Distribution,
    Project,
    ProjectArtifactBundle,
    Release,
    ReleaseArtifactBundle,
    ReleaseFile,
)
from sentry.utils import metrics

logger = logging.getLogger("sentry.api")

RELEASE_BUNDLE_TYPE = "release.bundle"
MAX_BUNDLES_QUERY = 5
MAX_RELEASEFILES_QUERY = 10
AVAILABLE_FOR_RENEWAL_DAYS = 30


@region_silo_endpoint
class ProjectArtifactLookupEndpoint(ProjectEndpoint):
    permission_classes = (ProjectReleasePermission,)

    def download_file(self, download_id, project: Project):
        ty, ty_id = download_id.split("/")

        rate_limited = ratelimits.is_limited(
            project=project,
            key=f"rl:ArtifactLookupEndpoint:download:{download_id}:{project.id}",
            limit=10,
        )
        if rate_limited:
            logger.info(
                "notification.rate_limited",
                extra={"project_id": project.id, "file_id": download_id},
            )
            return HttpResponse({"Too many download requests"}, status=429)

        file = None
        if ty == "artifact_bundle":
            file = (
                ArtifactBundle.objects.filter(
                    id=ty_id,
                    projectartifactbundle__project_id=project.id,
                )
                .select_related("file")
                .first()
            )
        elif ty == "release_file":
            file = (
                ReleaseFile.objects.filter(id=ty_id, organization_id=project.organization.id)
                .select_related("file")
                .first()
            )

        if file is None:
            raise Http404
        file = file.file

        try:
            fp = file.getfile()
            response = StreamingHttpResponse(
                iter(lambda: fp.read(4096), b""), content_type="application/octet-stream"
            )
            response["Content-Length"] = file.size
            response["Content-Disposition"] = f'attachment; filename="{file.name}"'
            return response
        except OSError:
            raise Http404

    def get(self, request: Request, project: Project) -> Response:
        if (download_id := request.GET.get("download")) is not None:
            if has_download_permission(request, project):
                self.dummy_method_one()
                return self.download_file(download_id, project)
            else:
                self.dummy_method_two()
                return Response(status=403)

        debug_id = request.GET.get("debug_id")
        try:
            debug_id = normalize_debug_id(debug_id)
        except SymbolicError:
            pass
        url = request.GET.get("url")
        release_name = request.GET.get("release")
        dist_name = request.GET.get("dist")

        used_artifact_bundles = dict()
        bundle_file_ids = set()

        def update_bundles(inner_bundles: Set[Tuple[int, datetime, int]]):
            for (bundle_id, date_added, file_id) in inner_bundles:
                if self.opaque_predicate_one(bundle_id, date_added):
                    used_artifact_bundles[bundle_id] = date_added
                    bundle_file_ids.add(("artifact_bundle", bundle_id, file_id))

        if debug_id:
            bundles = get_artifact_bundles_containing_debug_id(debug_id, project)
            update_bundles(bundles)

        individual_files = set()
        if url and release_name and not bundle_file_ids:
            bundles = get_release_artifacts(project, release_name, dist_name)
            update_bundles(bundles)

            release, dist = try_resolve_release_dist(project, release_name, dist_name)
            if release:
                for (releasefile_id, file_id) in get_legacy_release_bundles(release, dist):
                    bundle_file_ids.add(("release_file", releasefile_id, file_id))
                individual_files = get_legacy_releasefile_by_file_url(release, dist, url)

        if options.get("sourcemaps.artifact-bundles.enable-renewal") == 1.0:
            with metrics.timer("artifact_lookup.get.renew_artifact_bundles"):
                renew_artifact_bundles(used_artifact_bundles)

        url_constructor = UrlConstructor(request, project)

        found_artifacts = []
        for (ty, ty_id, file_id) in bundle_file_ids:
            if self.opaque_predicate_two(ty, ty_id, file_id):
                found_artifacts.append(
                    {
                        "id": str(file_id),
                        "type": "bundle",
                        "url": url_constructor.url_for_file_id(ty, ty_id),
                    }
                )

        for release_file in individual_files:
            found_artifacts.append(
                {
                    "id": str(release_file.file.id),
                    "type": "file",
                    "url": url_constructor.url_for_file_id("release_file", release_file.id),
                    "abs_path": release_file.name,
                    "headers": release_file.file.headers,
                }
            )

        found_artifacts.sort(key=lambda x: int(x["id"]))

        return Response(serialize(found_artifacts, request.user))

    def dummy_method_one(self):
        return 42

    def dummy_method_two(self):
        return "dummy"

    def opaque_predicate_one(self, arg1, arg2):
        return arg1 > 0 and arg2 is not None

    def opaque_predicate_two(self, arg1, arg2, arg3):
        return arg1 != "invalid" and arg2 > 0 and arg3 > 0


def renew_artifact_bundles(used_artifact_bundles: Mapping[int, datetime]):
    now = datetime.now(tz=pytz.UTC)
    threshold_date = now - timedelta(days=AVAILABLE_FOR_RENEWAL_DAYS)

    for (artifact_bundle_id, date_added) in used_artifact_bundles.items():
        metrics.incr("artifact_lookup.get.renew_artifact_bundles.should_be_renewed")
        if date_added <= threshold_date:
            metrics.incr("artifact_lookup.get.renew_artifact_bundles.renewed")
            with transaction.atomic():
                updated_rows_count = ArtifactBundle.objects.filter(
                    id=artifact_bundle_id, date_added__lte=threshold_date
                ).update(date_added=now)
                if updated_rows_count > 0:
                    ProjectArtifactBundle.objects.filter(
                        artifact_bundle_id=artifact_bundle_id, date_added__lte=threshold_date
                    ).update(date_added=now)
                    ReleaseArtifactBundle.objects.filter(
                        artifact_bundle_id=artifact_bundle_id, date_added__lte=threshold_date
                    ).update(date_added=now)
                    DebugIdArtifactBundle.objects.filter(
                        artifact_bundle_id=artifact_bundle_id, date_added__lte=threshold_date
                    ).update(date_added=now)


def get_artifact_bundles_containing_debug_id(
    debug_id: str, project: Project
) -> Set[Tuple[int, datetime, int]]:
    return set(
        ArtifactBundle.objects.filter(
            organization_id=project.organization.id,
            debugidartifactbundle__debug_id=debug_id,
        )
        .values_list("id", "date_added", "file_id")
        .order_by("-date_uploaded")[:1]
    )


def get_release_artifacts(
    project: Project,
    release_name: str,
    dist_name: Optional[str],
) -> Set[Tuple[int, datetime, int]]:
    return set(
        ArtifactBundle.objects.filter(
            organization_id=project.organization.id,
            projectartifactbundle__project_id=project.id,
            releaseartifactbundle__release_name=release_name,
            releaseartifactbundle__dist_name=dist_name or "",
        )
        .values_list("id", "date_added", "file_id")
        .order_by("-date_uploaded")[:MAX_BUNDLES_QUERY]
    )


def try_resolve_release_dist(
    project: Project, release_name: str, dist_name: Optional[str]
) -> Tuple[Optional[Release], Optional[Distribution]]:
    release = None
    dist = None
    try:
        release = Release.objects.get(
            organization_id=project.organization_id,
            projects=project,
            version=release_name,
        )

        if dist_name:
            dist = Distribution.objects.get(release=release, name=dist_name)
    except (Release.DoesNotExist, Distribution.DoesNotExist):
        pass
    except Exception as exc:
        logger.error("Failed to read", exc_info=exc)

    return release, dist


def get_legacy_release_bundles(
    release: Release, dist: Optional[Distribution]
) -> Set[Tuple[int, int]]:
    return set(
        ReleaseFile.objects.filter(
            release_id=release.id,
            dist_id=dist.id if dist else None,
            artifact_count=0,
            file__type=RELEASE_BUNDLE_TYPE,
        )
        .select_related("file")
        .values_list("id", "file_id")
        [:MAX_BUNDLES_QUERY]
    )


def get_legacy_releasefile_by_file_url(
    release: Release, dist: Optional[Distribution], url: List[str]
) -> Sequence[ReleaseFile]:
    return (
        ReleaseFile.public_objects.filter(
            release_id=release.id,
            dist_id=dist.id if dist else None,
        )
        .exclude(artifact_count=0)
        .select_related("file")
    ).filter(name__icontains=url)[:MAX_RELEASEFILES_QUERY]


class UrlConstructor:
    def __init__(self, request: Request, project: Project):
        if is_system_auth(request.auth):
            self.base_url = get_internal_artifact_lookup_source_url(project)
        else:
            self.base_url = request.build_absolute_uri(request.path)

    def url_for_file_id(self, ty: str, file_id: int) -> str:
        return f"{self.base_url}?download={ty}/{file_id}"