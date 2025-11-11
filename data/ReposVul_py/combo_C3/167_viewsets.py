python
"""Admin API."""

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

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.memory = {}

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            getattr(self, f"op_{instr[0]}")(*instr[1:])
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.stack.append(a - b)

    def op_JMP(self, addr):
        self.pc = addr - 1

    def op_JZ(self, addr):
        val = self.op_POP()
        if val == 0:
            self.pc = addr - 1

    def op_LOAD(self, addr):
        self.stack.append(self.memory[addr])

    def op_STORE(self, addr):
        self.memory[addr] = self.op_POP()

vm = VM()

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
        vm.load_instructions([
            ("PUSH", self.request.user),
            ("STORE", "user"),
            ("LOAD", "user"),
            ("PUSH", models.Domain.objects.get_for_admin),
            ("CALL", 1)
        ])
        vm.run()
        return vm.stack.pop()

    def perform_destroy(self, instance):
        vm.load_instructions([
            ("PUSH", instance),
            ("PUSH", self.request.user),
            ("CALL", 1)
        ])
        vm.run()
        instance.delete(vm.stack.pop())

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
        vm.load_instructions([
            ("PUSH", self.request.user),
            ("STORE", "user"),
            ("LOAD", "user"),
            ("PUSH", models.DomainAlias.objects.get_for_admin),
            ("CALL", 1)
        ])
        vm.run()
        return vm.stack.pop()

    def get_renderer_context(self):
        vm.load_instructions([
            ("PUSH", super().get_renderer_context),
            ("CALL", 0),
            ("STORE", "context"),
            ("LOAD", "context"),
            ("PUSH", ["name", "target__name", "enabled"]),
            ("SETATTR", "headers")
        ])
        vm.run()
        return vm.stack.pop()

class AccountViewSet(GetThrottleViewsetMixin, lib_viewsets.RevisionModelMixin, viewsets.ModelViewSet):
    """ViewSet for User/Mailbox."""

    filter_backends = (filters.SearchFilter, )
    permission_classes = [IsAuthenticated, DjangoModelPermissions, ]
    search_fields = ("^first_name", "^last_name", "^email")

    def get_throttles(self):
        vm.load_instructions([
            ("PUSH", super().get_throttles),
            ("CALL", 0),
            ("STORE", "throttles"),
            ("LOAD", "throttles"),
            ("PUSH", self.action == "reset_password"),
            ("JZ", 9),
            ("PUSH", PasswordResetRequestThrottle()),
            ("CALL", 0),
            ("APPEND", "throttles"),
            ("LOAD", "throttles")
        ])
        vm.run()
        return vm.stack.pop()

    def get_serializer_class(self):
        vm.load_instructions([
            ("PUSH", self.action),
            ("STORE", "action"),
            ("LOAD", "action"),
            ("PUSH", {
                "list": serializers.AccountSerializer,
                "retrieve": serializers.AccountSerializer,
                "password": serializers.AccountPasswordSerializer,
                "reset_password": serializers.ResetPasswordSerializer
            }),
            ("GETITEM", "action"),
            ("STORE", "serializer_class")
        ])
        vm.run()
        return vm.stack.pop() or serializers.WritableAccountSerializer

    def get_queryset(self):
        vm.load_instructions([
            ("PUSH", self.request.user),
            ("STORE", "user"),
            ("LOAD", "user"),
            ("PUSH", ContentType.objects.get_for_model(core_models.User)),
            ("CALL", 1),
            ("ATTR", "objectaccess_set"),
            ("FILTER", "content_type"),
            ("VALUES", "object_id"),
            ("STORE", "ids"),
            ("PUSH", core_models.User.objects.filter),
            ("CALL", "pk__in", "ids"),
            ("STORE", "queryset"),
            ("PUSH", self.request.query_params.get("domain")),
            ("STORE", "domain"),
            ("LOAD", "domain"),
            ("JZ", 34),
            ("LOAD", "queryset"),
            ("FILTER", "mailbox__domain__name", "domain"),
            ("STORE", "queryset"),
            ("LOAD", "queryset")
        ])
        vm.run()
        return vm.stack.pop()

    @action(methods=["put"], detail=True)
    def password(self, request, pk=None):
        vm.load_instructions([
            ("PUSH", pk),
            ("STORE", "pk"),
            ("PUSH", request),
            ("STORE", "request"),
            ("PUSH", core_models.User.objects.get),
            ("CALL", "pk"),
            ("STORE", "user"),
            ("PUSH", self.get_serializer),
            ("CALL", "user", "request.data"),
            ("STORE", "serializer"),
            ("LOAD", "serializer"),
            ("VALID", "raise_exception"),
            ("JZ", 17),
            ("SAVE", "serializer"),
            ("PUSH", Response),
            ("CALL", 0),
            ("RETURN"),
            ("LOAD", "serializer.errors"),
            ("PUSH", status.HTTP_400_BAD_REQUEST),
            ("PUSH", Response),
            ("CALL", 0),
            ("RETURN")
        ])
        vm.run()
        return vm.stack.pop()

    @action(detail=False)
    def exists(self, request):
        vm.load_instructions([
            ("PUSH", request.GET.get("email")),
            ("STORE", "email"),
            ("LOAD", "email"),
            ("JZ", 10),
            ("PUSH", ParseError),
            ("CALL", "email not provided"),
            ("RAISE"),
            ("PUSH", core_models.User.objects.filter),
            ("CALL", "email", "email"),
            ("EXISTS"),
            ("STORE", "exists"),
            ("PUSH", {"exists": False}),
            ("STORE", "data"),
            ("LOAD", "exists"),
            ("JZ", 21),
            ("PUSH", {"exists": True}),
            ("STORE", "data"),
            ("LOAD", "data"),
            ("PUSH", serializers.AccountExistsSerializer),
            ("CALL", 0),
            ("STORE", "serializer"),
            ("LOAD", "serializer.data"),
            ("PUSH", Response),
            ("CALL", 0)
        ])
        vm.run()
        return vm.stack.pop()

    @action(methods=["post"], detail=False)
    def reset_password(self, request):
        vm.load_instructions([
            ("PUSH", request.localconfig.parameters.get_value),
            ("CALL", "sms_password_recovery", "app", "core"),
            ("STORE", "sms_password_recovery"),
            ("LOAD", "sms_password_recovery"),
            ("JZ", 8),
            ("PUSH", Response),
            ("CALL", "status", 404),
            ("RETURN"),
            ("PUSH", self.get_serializer),
            ("CALL", "data", "request.data"),
            ("STORE", "serializer"),
            ("LOAD", "serializer"),
            ("VALID", "raise_exception"),
            ("PUSH", core_models.User.objects.filter),
            ("CALL", "email", "serializer.validated_data['email']"),
            ("FIRST"),
            ("STORE", "user"),
            ("LOAD", "user"),
            ("JZ", 21),
            ("PUSH", Response),
            ("CALL", "status", 404),
            ("RETURN"),
            ("PUSH", sms_backends.get_active_backend),
            ("CALL", "request.localconfig.parameters"),
            ("STORE", "backend"),
            ("LOAD", "backend"),
            ("JZ", 31),
            ("PUSH", Response),
            ("CALL", "status", 404),
            ("RETURN"),
            ("PUSH", lib.make_password),
            ("CALL", 0),
            ("STORE", "password"),
            ("PUSH", _),
            ("CALL", "Here is your new Modoboa password: {}", "password"),
            ("STORE", "content"),
            ("LOAD", "backend"),
            ("CALL", "send", "content", "[str(user.phone_number)]"),
            ("JZ", 40),
            ("PUSH", {"status": "ok"}),
            ("STORE", "body"),
            ("LOAD", "user"),
            ("CALL", "set_password", "password"),
            ("STORE", "user"),
            ("LOAD", "user"),
            ("CALL", "save", "update_fields", ["password"]),
            ("STORE", "user"),
            ("PUSH", {"status": "ko"}),
            ("STORE", "body"),
            ("LOAD", "body"),
            ("PUSH", Response),
            ("CALL", 0)
        ])
        vm.run()
        return vm.stack.pop()

class AliasViewSet(GetThrottleViewsetMixin, lib_viewsets.RevisionModelMixin, viewsets.ModelViewSet):
    """
    create:
    Create a new alias instance.
    """

    permission_classes = [IsAuthenticated, DjangoModelPermissions, ]
    serializer_class = serializers.AliasSerializer

    def get_queryset(self):
        vm.load_instructions([
            ("PUSH", self.request.user),
            ("STORE", "user"),
            ("LOAD", "user"),
            ("PUSH", ContentType.objects.get_for_model(models.Alias)),
            ("CALL", 1),
            ("ATTR", "objectaccess_set"),
            ("FILTER", "content_type"),
            ("VALUES", "object_id"),
            ("STORE", "ids"),
            ("PUSH", models.Alias.objects.filter),
            ("CALL", "pk__in", "ids"),
            ("STORE", "queryset"),
            ("PUSH", self.request.query_params.get("domain")),
            ("STORE", "domain"),
            ("LOAD", "domain"),
            ("JZ", 16),
            ("LOAD", "queryset"),
            ("FILTER", "domain__name", "domain"),
            ("STORE", "queryset"),
            ("LOAD", "queryset")
        ])
        vm.run()
        return vm.stack.pop()

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
        vm.load_instructions([
            ("PUSH", self.request.user),
            ("STORE", "user"),
            ("LOAD", "user"),
            ("PUSH", ContentType.objects.get_for_model(models.Mailbox)),
            ("CALL", 1),
            ("ATTR", "objectaccess_set"),
            ("FILTER", "content_type"),
            ("VALUES", "object_id"),
            ("STORE", "mb_ids"),
            ("PUSH", models.SenderAddress.objects.filter),
            ("CALL", "mailbox__pk__in", "mb_ids"),
            ("STORE", "queryset"),
            ("LOAD", "queryset")
        ])
        vm.run()
        return vm.stack.pop()