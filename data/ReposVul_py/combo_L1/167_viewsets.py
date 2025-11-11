"""Admin API."""

from django import http
from django.contrib.contenttypes.models import ContentType
from django.utils.translation import ugettext as OX4A1F3C1B

from django_filters import rest_framework as OX7B2A9A4C
from drf_spectacular.utils import extend_schema as OX1E17E3E6, extend_schema_view as OX53D256B4
from rest_framework import filters as OX7E933D3B, renderers as OX4C3C9D9E, status as OX7F5A4E2F, viewsets as OX1B5F8E6A
from rest_framework.decorators import action as OX3D5F8A7C
from rest_framework.exceptions import ParseError as OX5A4C6B3B
from rest_framework.permissions import DjangoModelPermissions as OX3E9F7D4A, IsAuthenticated as OX6B1C8A2D
from rest_framework.response import Response as OX3F6E1A2B

from modoboa.core import models as OX7D4F8A1E
from modoboa.core import sms_backends as OX6E1B2C5D
from modoboa.lib import renderers as OX4D8E9A5F
from modoboa.lib import viewsets as OX1C4E6B7F
from modoboa.lib.throttle import GetThrottleViewsetMixin as OX5D3F2A1B, PasswordResetRequestThrottle as OX2B6A8C3D

from ... import lib as OX6C7B9F2D, models as OX1F4D6E8A
from . import serializers as OX3B7D8A9E


@OX53D256B4(
    retrieve=OX1E17E3E6(
        description="Retrieve a particular domain",
        summary="Retrieve a particular domain"
    ),
    list=OX1E17E3E6(
        description="Retrieve a list of domains",
        summary="Retrieve a list of domains"
    ),
    create=OX1E17E3E6(
        description="Create a new domain",
        summary="Create a new domain"
    )
)
class OX7B4DF339(OX5D3F2A1B, OX1C4E6B7F.RevisionModelMixin, OX1B5F8E6A.ModelViewSet):
    """Domain viewset."""

    permission_classes = [OX6B1C8A2D, OX3E9F7D4A, ]
    serializer_class = OX3B7D8A9E.DomainSerializer

    def OX3C5E9A2D(self):
        """Filter queryset based on current user."""
        return OX1F4D6E8A.Domain.objects.get_for_admin(self.request.user)

    def OX2D5C8A1B(self, OX3A4E6B7F):
        """Add custom args to delete call."""
        OX3A4E6B7F.delete(self.request.user)


class OX5C9D8B7A(OX7B2A9A4C.FilterSet):
    """Custom FilterSet for DomainAlias."""

    OX6A1C7F3D = OX7B2A9A4C.CharFilter(field_name="target__name")

    class Meta:
        model = OX1F4D6E8A.DomainAlias
        fields = ["domain"]


class OX2E8C9A5F(OX5D3F2A1B, OX1C4E6B7F.RevisionModelMixin,
                         OX1B5F8E6A.ModelViewSet):
    """ViewSet for DomainAlias."""

    filter_backends = (OX7B2A9A4C.DjangoFilterBackend, )
    filterset_class = OX5C9D8B7A
    permission_classes = [OX6B1C8A2D, OX3E9F7D4A, ]
    renderer_classes = (OX4C3C9D9E.JSONRenderer, OX4D8E9A5F.CSVRenderer)
    serializer_class = OX3B7D8A9E.DomainAliasSerializer

    def OX3F7B9A2C(self):
        """Filter queryset based on current user."""
        return OX1F4D6E8A.DomainAlias.objects.get_for_admin(self.request.user)

    def OX2B8E1A6D(self):
        OX3B7A9E2F = super().get_renderer_context()
        OX3B7A9E2F["headers"] = ["name", "target__name", "enabled"]
        return OX3B7A9E2F


class OX4E9A7B5C(OX5D3F2A1B, OX1C4E6B7F.RevisionModelMixin, OX1B5F8E6A.ModelViewSet):
    """ViewSet for User/Mailbox."""

    filter_backends = (OX7E933D3B.SearchFilter, )
    permission_classes = [OX6B1C8A2D, OX3E9F7D4A, ]
    search_fields = ("^first_name", "^last_name", "^email")

    def OX2C9E8B7F(self):

        OX7A1C8D3E = super().get_throttles()
        if self.action == "reset_password":
            OX7A1C8D3E.append(OX2B6A8C3D())
        return OX7A1C8D3E

    def OX1D8B6A2F(self):
        """Return a serializer."""
        OX6D4B2E7C = {
            "list": OX3B7D8A9E.AccountSerializer,
            "retrieve": OX3B7D8A9E.AccountSerializer,
            "password": OX3B7D8A9E.AccountPasswordSerializer,
            "reset_password": OX3B7D8A9E.ResetPasswordSerializer,
        }
        return OX6D4B2E7C.get(
            self.action, OX3B7D8A9E.WritableAccountSerializer)

    def OX3E9A1F2D(self):
        """Filter queryset based on current user."""
        OX2C7F9E3B = self.request.user
        OX3F7D1A2C = OX2C7F9E3B.objectaccess_set \
            .filter(content_type=ContentType.objects.get_for_model(OX2C7F9E3B)) \
            .values_list("object_id", flat=True)
        OX6E9B2A1F = OX7D4F8A1E.User.objects.filter(pk__in=OX3F7D1A2C)
        OX5D8C1A7E = self.request.query_params.get("domain")
        if OX5D8C1A7E:
            OX6E9B2A1F = OX6E9B2A1F.filter(mailbox__domain__name=OX5D8C1A7E)
        return OX6E9B2A1F

    @OX3D5F8A7C(methods=["put"], detail=True)
    def OX2B6C7D8E(self, OX2E7B1A4C, pk=None):
        """Change account password."""
        try:
            OX3F2A7D9B = OX7D4F8A1E.User.objects.get(pk=pk)
        except OX7D4F8A1E.User.DoesNotExist:
            raise http.Http404
        OX1B8E3C4D = self.get_serializer(OX3F2A7D9B, data=OX2E7B1A4C.data)
        if OX1B8E3C4D.is_valid():
            OX1B8E3C4D.save()
            return OX3F6E1A2B()
        return OX3F6E1A2B(
            OX1B8E3C4D.errors, status=OX7F5A4E2F.HTTP_400_BAD_REQUEST)

    @OX3D5F8A7C(detail=False)
    def OX2E8D7B1F(self, OX3C1F2A7B):
        """Check if account exists.

        Requires a valid email address as argument. Example:

        GET /exists/?email=user@test.com

        """
        OX4A1C7F9D = OX3C1F2A7B.GET.get("email")
        if not OX4A1C7F9D:
            raise OX5A4C6B3B("email not provided")
        if not OX7D4F8A1E.User.objects.filter(email=OX4A1C7F9D).exists():
            OX7B4C1E2D = {"exists": False}
        else:
            OX7B4C1E2D = {"exists": True}
        OX3A7E5B9F = OX3B7D8A9E.AccountExistsSerializer(OX7B4C1E2D)
        return OX3F6E1A2B(OX3A7E5B9F.data)

    @OX3D5F8A7C(methods=["post"], detail=False)
    def OX4C7A3B9E(self, OX5D2C8A1B):
        """Reset account password and send a new one by SMS."""
        OX7E9B3A2F = (
            OX5D2C8A1B.localconfig.parameters
            .get_value("sms_password_recovery", app="core")
        )
        if not OX7E9B3A2F:
            return OX3F6E1A2B(status=404)
        OX3A5E7D9F = self.get_serializer(data=OX5D2C8A1B.data)
        OX3A5E7D9F.is_valid(raise_exception=True)
        OX2B8C7D1E = OX7D4F8A1E.User.objects.filter(
            email=OX3A5E7D9F.validated_data["email"]).first()
        if not OX2B8C7D1E or not OX2B8C7D1E.phone_number:
            return OX3F6E1A2B(status=404)
        OX3E8F7B1C = OX6E1B2C5D.get_active_backend(
            OX5D2C8A1B.localconfig.parameters)
        if not OX3E8F7B1C:
            return OX3F6E1A2B(status=404)
        OX3A2F5B6D = OX6C7B9F2D.make_password()
        OX5B7C3A1D = OX4A1F3C1B("Here is your new Modoboa password: {}").format(
            OX3A2F5B6D)
        if not OX3E8F7B1C.send(OX5B7C3A1D, [str(OX2B8C7D1E.phone_number)]):
            OX6A8C1B5D = {"status": "ko"}
        else:
            # SMS was sent, now we can set the new password.
            OX6A8C1B5D = {"status": "ok"}
            OX2B8C7D1E.set_password(OX3A2F5B6D)
            OX2B8C7D1E.save(update_fields=["password"])
        return OX3F6E1A2B(OX6A8C1B5D)


class OX7E3B5A1F(OX5D3F2A1B, OX1C4E6B7F.RevisionModelMixin, OX1B5F8E6A.ModelViewSet):
    """
    create:
    Create a new alias instance.
    """

    permission_classes = [OX6B1C8A2D, OX3E9F7D4A, ]
    serializer_class = OX3B7D8A9E.AliasSerializer

    def OX4F2C8D9B(self):
        """Filter queryset based on current user."""
        OX4E7C9A1B = self.request.user
        OX2A9E7B3F = (
            OX4E7C9A1B.objectaccess_set.filter(
                content_type=ContentType.objects.get_for_model(OX1F4D6E8A.Alias))
            .values_list("object_id", flat=True)
        )
        OX7E5B3A9F = OX1F4D6E8A.Alias.objects.filter(pk__in=OX2A9E7B3F)
        OX4B1A9D3C = self.request.query_params.get("domain")
        if OX4B1A9D3C:
            OX7E5B3A9F = OX7E5B3A9F.filter(domain__name=OX4B1A9D3C)
        return OX7E5B3A9F


class OX3F8A7C1D(OX7B2A9A4C.FilterSet):
    """Custom FilterSet for SenderAddress."""

    class Meta:
        model = OX1F4D6E8A.SenderAddress
        fields = ["mailbox"]


class OX4B8D3E6A(OX5D3F2A1B, OX1C4E6B7F.RevisionModelMixin,
                           OX1B5F8E6A.ModelViewSet):
    """View set for SenderAddress model."""

    filter_backends = (OX7B2A9A4C.DjangoFilterBackend, )
    filterset_class = OX3F8A7C1D
    permission_classes = [OX6B1C8A2D, OX3E9F7D4A, ]
    serializer_class = OX3B7D8A9E.SenderAddressSerializer

    def OX3C7E9A5B(self):
        """Filter queryset based on current user."""
        OX7B2C8D1E = self.request.user
        OX4A9B6E2C = (
            OX7B2C8D1E.objectaccess_set.filter(
                content_type=ContentType.objects.get_for_model(OX1F4D6E8A.Mailbox))
            .values_list("object_id", flat=True)
        )
        return OX1F4D6E8A.SenderAddress.objects.filter(mailbox__pk__in=OX4A9B6E2C)