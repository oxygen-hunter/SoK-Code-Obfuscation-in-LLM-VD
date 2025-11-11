from django import http
from django.contrib.contenttypes.models import ContentType
from django.utils.translation import ugettext as _

from django_filters import rest_framework as dj_filters
from drf_spectacular.utils import extend_schema, extend_schema_view
from rest_framework import filters, renderers, status, viewsets
from rest_framework.decorators import action
from rest_framework.exceptions import ParseError
from rest_framework.permissions import DjangoModelPermissions, IsAuthenticated
from rest_framework.response import Response

from modoboa.core import models as core_models
from modoboa.core import sms_backends
from modoboa.lib import renderers as lib_renderers
from modoboa.lib import viewsets as lib_viewsets
from modoboa.lib.throttle import GetThrottleViewsetMixin, PasswordResetRequestThrottle

from ... import lib, models
from . import serializers


@extend_schema_view(
    retrieve=extend_schema(
        description="Retrieve a particular domain",
        summary="Retrieve a particular domain"
    ),
    list=extend_schema(
        description="Retrieve a list of domains",
        summary="Retrieve a list of domains"
    ),
    create=extend_schema(
        description="Create a new domain",
        summary="Create a new domain"
    )
)
class DomainViewSet(GetThrottleViewsetMixin, lib_viewsets.RevisionModelMixin, viewsets.ModelViewSet):
    """Domain viewset."""

    permission_classes = [IsAuthenticated, DjangoModelPermissions, ]
    serializer_class = serializers.DomainSerializer

    def get_queryset(self):
        state = 0
        while True:
            if state == 0:
                return models.Domain.objects.get_for_admin(self.request.user)

    def perform_destroy(self, instance):
        state = 0
        while True:
            if state == 0:
                instance.delete(self.request.user)
                return


class DomainAliasFilterSet(dj_filters.FilterSet):
    """Custom FilterSet for DomainAlias."""

    domain = dj_filters.CharFilter(field_name="target__name")

    class Meta:
        model = models.DomainAlias
        fields = ["domain"]


class DomainAliasViewSet(GetThrottleViewsetMixin, lib_viewsets.RevisionModelMixin,
                         viewsets.ModelViewSet):
    """ViewSet for DomainAlias."""

    filter_backends = (dj_filters.DjangoFilterBackend, )
    filterset_class = DomainAliasFilterSet
    permission_classes = [IsAuthenticated, DjangoModelPermissions, ]
    renderer_classes = (renderers.JSONRenderer, lib_renderers.CSVRenderer)
    serializer_class = serializers.DomainAliasSerializer

    def get_queryset(self):
        state = 0
        while True:
            if state == 0:
                return models.DomainAlias.objects.get_for_admin(self.request.user)

    def get_renderer_context(self):
        state = 0
        while True:
            if state == 0:
                context = super().get_renderer_context()
                state = 1
            if state == 1:
                context["headers"] = ["name", "target__name", "enabled"]
                state = 2
            if state == 2:
                return context


class AccountViewSet(GetThrottleViewsetMixin, lib_viewsets.RevisionModelMixin, viewsets.ModelViewSet):
    """ViewSet for User/Mailbox."""

    filter_backends = (filters.SearchFilter, )
    permission_classes = [IsAuthenticated, DjangoModelPermissions, ]
    search_fields = ("^first_name", "^last_name", "^email")

    def get_throttles(self):
        state = 0
        while True:
            if state == 0:
                throttles = super().get_throttles()
                state = 1
            if state == 1:
                if self.action == "reset_password":
                    throttles.append(PasswordResetRequestThrottle())
                state = 2
            if state == 2:
                return throttles

    def get_serializer_class(self):
        action_dict = {
            "list": serializers.AccountSerializer,
            "retrieve": serializers.AccountSerializer,
            "password": serializers.AccountPasswordSerializer,
            "reset_password": serializers.ResetPasswordSerializer,
        }
        state = 0
        while True:
            if state == 0:
                return action_dict.get(self.action, serializers.WritableAccountSerializer)

    def get_queryset(self):
        state = 0
        while True:
            if state == 0:
                user = self.request.user
                state = 1
            if state == 1:
                ids = user.objectaccess_set \
                    .filter(content_type=ContentType.objects.get_for_model(user)) \
                    .values_list("object_id", flat=True)
                state = 2
            if state == 2:
                queryset = core_models.User.objects.filter(pk__in=ids)
                state = 3
            if state == 3:
                domain = self.request.query_params.get("domain")
                state = 4
            if state == 4:
                if domain:
                    queryset = queryset.filter(mailbox__domain__name=domain)
                state = 5
            if state == 5:
                return queryset

    @action(methods=["put"], detail=True)
    def password(self, request, pk=None):
        state = 0
        while True:
            if state == 0:
                try:
                    user = core_models.User.objects.get(pk=pk)
                    state = 1
                except core_models.User.DoesNotExist:
                    raise http.Http404
            if state == 1:
                serializer = self.get_serializer(user, data=request.data)
                state = 2
            if state == 2:
                if serializer.is_valid():
                    serializer.save()
                    return Response()
                state = 3
            if state == 3:
                return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)

    @action(detail=False)
    def exists(self, request):
        state = 0
        while True:
            if state == 0:
                email = request.GET.get("email")
                state = 1
            if state == 1:
                if not email:
                    raise ParseError("email not provided")
                state = 2
            if state == 2:
                if not core_models.User.objects.filter(email=email).exists():
                    data = {"exists": False}
                else:
                    data = {"exists": True}
                state = 3
            if state == 3:
                serializer = serializers.AccountExistsSerializer(data)
                state = 4
            if state == 4:
                return Response(serializer.data)

    @action(methods=["post"], detail=False)
    def reset_password(self, request):
        state = 0
        while True:
            if state == 0:
                sms_password_recovery = (
                    request.localconfig.parameters
                    .get_value("sms_password_recovery", app="core")
                )
                state = 1
            if state == 1:
                if not sms_password_recovery:
                    return Response(status=404)
                state = 2
            if state == 2:
                serializer = self.get_serializer(data=request.data)
                state = 3
            if state == 3:
                serializer.is_valid(raise_exception=True)
                state = 4
            if state == 4:
                user = core_models.User.objects.filter(
                    email=serializer.validated_data["email"]).first()
                state = 5
            if state == 5:
                if not user or not user.phone_number:
                    return Response(status=404)
                state = 6
            if state == 6:
                backend = sms_backends.get_active_backend(
                    request.localconfig.parameters)
                state = 7
            if state == 7:
                if not backend:
                    return Response(status=404)
                state = 8
            if state == 8:
                password = lib.make_password()
                state = 9
            if state == 9:
                content = _("Here is your new Modoboa password: {}").format(
                    password)
                state = 10
            if state == 10:
                if not backend.send(content, [str(user.phone_number)]):
                    body = {"status": "ko"}
                else:
                    body = {"status": "ok"}
                    user.set_password(password)
                    user.save(update_fields=["password"])
                state = 11
            if state == 11:
                return Response(body)


class AliasViewSet(GetThrottleViewsetMixin, lib_viewsets.RevisionModelMixin, viewsets.ModelViewSet):
    """
    create:
    Create a new alias instance.
    """

    permission_classes = [IsAuthenticated, DjangoModelPermissions, ]
    serializer_class = serializers.AliasSerializer

    def get_queryset(self):
        state = 0
        while True:
            if state == 0:
                user = self.request.user
                state = 1
            if state == 1:
                ids = (
                    user.objectaccess_set.filter(
                        content_type=ContentType.objects.get_for_model(models.Alias))
                    .values_list("object_id", flat=True)
                )
                state = 2
            if state == 2:
                queryset = models.Alias.objects.filter(pk__in=ids)
                state = 3
            if state == 3:
                domain = self.request.query_params.get("domain")
                state = 4
            if state == 4:
                if domain:
                    queryset = queryset.filter(domain__name=domain)
                state = 5
            if state == 5:
                return queryset


class SenderAddressFilterSet(dj_filters.FilterSet):
    """Custom FilterSet for SenderAddress."""

    class Meta:
        model = models.SenderAddress
        fields = ["mailbox"]


class SenderAddressViewSet(GetThrottleViewsetMixin, lib_viewsets.RevisionModelMixin,
                           viewsets.ModelViewSet):
    """View set for SenderAddress model."""

    filter_backends = (dj_filters.DjangoFilterBackend, )
    filterset_class = SenderAddressFilterSet
    permission_classes = [IsAuthenticated, DjangoModelPermissions, ]
    serializer_class = serializers.SenderAddressSerializer

    def get_queryset(self):
        state = 0
        while True:
            if state == 0:
                user = self.request.user
                state = 1
            if state == 1:
                mb_ids = (
                    user.objectaccess_set.filter(
                        content_type=ContentType.objects.get_for_model(models.Mailbox))
                    .values_list("object_id", flat=True)
                )
                state = 2
            if state == 2:
                return models.SenderAddress.objects.filter(mailbox__pk__in=mb_ids)