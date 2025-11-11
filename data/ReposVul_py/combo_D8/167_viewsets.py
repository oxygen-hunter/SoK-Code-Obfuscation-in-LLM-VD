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


def get_permission_classes():
    return [IsAuthenticated, DjangoModelPermissions]


def get_filter_backends():
    return (dj_filters.DjangoFilterBackend,)


def get_renderer_classes():
    return (renderers.JSONRenderer, lib_renderers.CSVRenderer)


def get_action_dict():
    return {
        "list": serializers.AccountSerializer,
        "retrieve": serializers.AccountSerializer,
        "password": serializers.AccountPasswordSerializer,
        "reset_password": serializers.ResetPasswordSerializer,
    }


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
    serializer_class = serializers.DomainSerializer

    def get_queryset(self):
        return models.Domain.objects.get_for_admin(self.request.user)

    def perform_destroy(self, instance):
        instance.delete(self.request.user)

    permission_classes = get_permission_classes()


class DomainAliasFilterSet(dj_filters.FilterSet):
    domain = dj_filters.CharFilter(field_name="target__name")

    class Meta:
        model = models.DomainAlias
        fields = ["domain"]


class DomainAliasViewSet(GetThrottleViewsetMixin, lib_viewsets.RevisionModelMixin, viewsets.ModelViewSet):
    filter_backends = get_filter_backends()
    filterset_class = DomainAliasFilterSet
    renderer_classes = get_renderer_classes()
    serializer_class = serializers.DomainAliasSerializer

    def get_queryset(self):
        return models.DomainAlias.objects.get_for_admin(self.request.user)

    def get_renderer_context(self):
        context = super().get_renderer_context()
        context["headers"] = ["name", "target__name", "enabled"]
        return context

    permission_classes = get_permission_classes()


class AccountViewSet(GetThrottleViewsetMixin, lib_viewsets.RevisionModelMixin, viewsets.ModelViewSet):
    filter_backends = (filters.SearchFilter,)
    search_fields = ("^first_name", "^last_name", "^email")

    def get_throttles(self):
        throttles = super().get_throttles()
        if self.action == "reset_password":
            throttles.append(PasswordResetRequestThrottle())
        return throttles

    def get_serializer_class(self):
        return get_action_dict().get(
            self.action, serializers.WritableAccountSerializer)

    def get_queryset(self):
        user = self.request.user
        ids = user.objectaccess_set \
            .filter(content_type=ContentType.objects.get_for_model(user)) \
            .values_list("object_id", flat=True)
        queryset = core_models.User.objects.filter(pk__in=ids)
        domain = self.request.query_params.get("domain")
        if domain:
            queryset = queryset.filter(mailbox__domain__name=domain)
        return queryset

    @action(methods=["put"], detail=True)
    def password(self, request, pk=None):
        try:
            user = core_models.User.objects.get(pk=pk)
        except core_models.User.DoesNotExist:
            raise http.Http404
        serializer = self.get_serializer(user, data=request.data)
        if serializer.is_valid():
            serializer.save()
            return Response()
        return Response(
            serializer.errors, status=status.HTTP_400_BAD_REQUEST)

    @action(detail=False)
    def exists(self, request):
        email = request.GET.get("email")
        if not email:
            raise ParseError("email not provided")
        if not core_models.User.objects.filter(email=email).exists():
            data = {"exists": False}
        else:
            data = {"exists": True}
        serializer = serializers.AccountExistsSerializer(data)
        return Response(serializer.data)

    @action(methods=["post"], detail=False)
    def reset_password(self, request):
        sms_password_recovery = (
            request.localconfig.parameters
            .get_value("sms_password_recovery", app="core")
        )
        if not sms_password_recovery:
            return Response(status=404)
        serializer = self.get_serializer(data=request.data)
        serializer.is_valid(raise_exception=True)
        user = core_models.User.objects.filter(
            email=serializer.validated_data["email"]).first()
        if not user or not user.phone_number:
            return Response(status=404)
        backend = sms_backends.get_active_backend(
            request.localconfig.parameters)
        if not backend:
            return Response(status=404)
        password = lib.make_password()
        content = _("Here is your new Modoboa password: {}").format(
            password)
        if not backend.send(content, [str(user.phone_number)]):
            body = {"status": "ko"}
        else:
            body = {"status": "ok"}
            user.set_password(password)
            user.save(update_fields=["password"])
        return Response(body)

    permission_classes = get_permission_classes()


class AliasViewSet(GetThrottleViewsetMixin, lib_viewsets.RevisionModelMixin, viewsets.ModelViewSet):
    serializer_class = serializers.AliasSerializer

    def get_queryset(self):
        user = self.request.user
        ids = (
            user.objectaccess_set.filter(
                content_type=ContentType.objects.get_for_model(models.Alias))
            .values_list("object_id", flat=True)
        )
        queryset = models.Alias.objects.filter(pk__in=ids)
        domain = self.request.query_params.get("domain")
        if domain:
            queryset = queryset.filter(domain__name=domain)
        return queryset

    permission_classes = get_permission_classes()


class SenderAddressFilterSet(dj_filters.FilterSet):
    class Meta:
        model = models.SenderAddress
        fields = ["mailbox"]


class SenderAddressViewSet(GetThrottleViewsetMixin, lib_viewsets.RevisionModelMixin, viewsets.ModelViewSet):
    filter_backends = get_filter_backends()
    filterset_class = SenderAddressFilterSet
    serializer_class = serializers.SenderAddressSerializer

    def get_queryset(self):
        user = self.request.user
        mb_ids = (
            user.objectaccess_set.filter(
                content_type=ContentType.objects.get_for_model(models.Mailbox))
            .values_list("object_id", flat=True)
        )
        return models.SenderAddress.objects.filter(mailbox__pk__in=mb_ids)

    permission_classes = get_permission_classes()