python
"""authentik multi-stage authentication engine"""
from datetime import timedelta
from django.contrib import messages
from django.http import HttpRequest, HttpResponse
from django.urls import reverse
from django.utils.http import urlencode
from django.utils.text import slugify
from django.utils.timezone import now
from django.utils.translation import gettext as _
from rest_framework.fields import CharField
from rest_framework.serializers import ValidationError

from authentik.flows.challenge import Challenge, ChallengeResponse, ChallengeTypes
from authentik.flows.models import FlowDesignation, FlowToken
from authentik.flows.planner import PLAN_CONTEXT_IS_RESTORED, PLAN_CONTEXT_PENDING_USER
from authentik.flows.stage import ChallengeStageView
from authentik.flows.views.executor import QS_KEY_TOKEN
from authentik.stages.email.models import EmailStage
from authentik.stages.email.tasks import send_mails
from authentik.stages.email.utils import TemplateEmailMessage

PLAN_CONTEXT_EMAIL_SENT = "email_sent"
PLAN_CONTEXT_EMAIL_OVERRIDE = "email"

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True

    def load(self, instructions):
        self.instructions = instructions

    def fetch(self):
        if self.pc < len(self.instructions):
            ins = self.instructions[self.pc]
            self.pc += 1
            return ins
        return None

    def eval(self, ins):
        op, *args = ins
        if op == "PUSH":
            self.stack.append(args[0])
        elif op == "POP":
            self.stack.pop()
        elif op == "ADD":
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)
        elif op == "SUB":
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)
        elif op == "JMP":
            self.pc = args[0]
        elif op == "JZ":
            if self.stack.pop() == 0:
                self.pc = args[0]
        elif op == "LOAD":
            self.stack.append(args[0])
        elif op == "STORE":
            index = args[0]
            self.stack[index] = self.stack.pop()
        elif op == "HALT":
            self.running = False

    def run(self):
        while self.running:
            ins = self.fetch()
            if ins:
                self.eval(ins)

class EmailChallenge(Challenge):
    component = CharField(default="ak-stage-email")

class EmailChallengeResponse(ChallengeResponse):
    component = CharField(default="ak-stage-email")

    def validate(self, attrs):
        vm = VM()
        vm.load([
            ("PUSH", "email-sent"),
            ("PUSH", "email-sent"),
            ("HALT",),
        ])
        vm.run()
        raise ValidationError(detail=vm.stack[0], code=vm.stack[1])

class EmailStageView(ChallengeStageView):
    response_class = EmailChallengeResponse

    def execute_vm(self, instructions):
        vm = VM()
        vm.load(instructions)
        vm.run()
        return vm.stack

    def get_full_url(self, **kwargs) -> str:
        result = self.execute_vm([
            ("LOAD", reverse("authentik_core:if-flow", kwargs={"flow_slug": self.executor.flow.slug})),
            ("LOAD", kwargs),
            ("HALT",),
        ])
        base_url = result[0]
        relative_url = f"{base_url}?{urlencode(result[1])}"
        return self.request.build_absolute_uri(relative_url)

    def get_token(self) -> FlowToken:
        result = self.execute_vm([
            ("LOAD", self.get_pending_user()),
            ("LOAD", self.executor.current_stage),
            ("LOAD", timedelta(minutes=self.executor.current_stage.token_expiry + 1)),
            ("HALT",),
        ])
        pending_user = result[0]
        current_stage = result[1]
        valid_delta = result[2]
        
        identifier = slugify(f"ak-email-stage-{current_stage.name}-{pending_user}")
        tokens = FlowToken.objects.filter(identifier=identifier)
        if not tokens.exists():
            return FlowToken.objects.create(
                expires=now() + valid_delta,
                user=pending_user,
                identifier=identifier,
                flow=self.executor.flow,
                _plan=FlowToken.pickle(self.executor.plan),
            )
        token = tokens.first()
        if token.is_expired:
            token.expire_action()
        return token

    def send_email(self):
        result = self.execute_vm([
            ("LOAD", self.get_pending_user()),
            ("LOAD", self.executor.flow.designation),
            ("LOAD", FlowDesignation.RECOVERY),
            ("HALT",),
        ])
        pending_user = result[0]
        flow_designation = result[1]
        recovery_designation = result[2]
        
        if not pending_user.pk and flow_designation == recovery_designation:
            return
        email = self.executor.plan.context.get(PLAN_CONTEXT_EMAIL_OVERRIDE, None)
        if not email:
            email = pending_user.email
        current_stage = self.executor.current_stage
        token = self.get_token()
        
        message = TemplateEmailMessage(
            subject=_(current_stage.subject),
            to=[email],
            language=pending_user.locale(self.request),
            template_name=current_stage.template,
            template_context={
                "url": self.get_full_url(**{QS_KEY_TOKEN: token.key}),
                "user": pending_user,
                "expires": token.expires,
            },
        )
        send_mails(current_stage, message)

    def get(self, request: HttpRequest, *args, **kwargs) -> HttpResponse:
        restore_token = self.executor.plan.context.get(PLAN_CONTEXT_IS_RESTORED, None)
        user = self.get_pending_user()
        if restore_token:
            if restore_token.user != user:
                self.logger.warning("Flow token for non-matching user, denying request")
                return self.executor.stage_invalid()
            messages.success(request, _("Successfully verified Email."))
            if self.executor.current_stage.activate_user_on_success:
                user.is_active = True
                user.save()
            return self.executor.stage_ok()
        if PLAN_CONTEXT_PENDING_USER not in self.executor.plan.context:
            self.logger.debug("No pending user")
            messages.error(self.request, _("No pending user."))
            return self.executor.stage_invalid()
        if PLAN_CONTEXT_EMAIL_SENT not in self.executor.plan.context:
            self.send_email()
            self.executor.plan.context[PLAN_CONTEXT_EMAIL_SENT] = True
        return super().get(request, *args, **kwargs)

    def get_challenge(self) -> Challenge:
        return EmailChallenge(data={"type": ChallengeTypes.NATIVE.value, "title": _("Email sent.")})

    def challenge_valid(self, response: ChallengeResponse) -> HttpResponse:
        return super().challenge_invalid(response)

    def challenge_invalid(self, response: ChallengeResponse) -> HttpResponse:
        if PLAN_CONTEXT_PENDING_USER not in self.executor.plan.context:
            messages.error(self.request, _("No pending user."))
            return super().challenge_invalid(response)
        self.send_email()
        return super().challenge_invalid(response)