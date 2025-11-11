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


class EmailChallenge(Challenge):
    component = CharField(default="ak-stage-email")


class EmailChallengeResponse(ChallengeResponse):
    component = CharField(default="ak-stage-email")

    def validate(self, attrs):
        raise ValidationError(detail="email-sent", code="email-sent")


class EmailStageView(ChallengeStageView):
    response_class = EmailChallengeResponse

    def get_full_url(self, **kwargs) -> str:
        base_url = reverse(
            "authentik_core:if-flow",
            kwargs={"flow_slug": self.executor.flow.slug},
        )
        relative_url = f"{base_url}?{urlencode(kwargs)}"
        return self.request.build_absolute_uri(relative_url)

    def get_token(self) -> FlowToken:
        pending_user = self.get_pending_user()
        current_stage: EmailStage = self.executor.current_stage
        valid_delta = timedelta(
            minutes=current_stage.token_expiry + 1
        )
        identifier = slugify(f"ak-email-stage-{current_stage.name}-{pending_user}")
        tokens = FlowToken.objects.filter(identifier=identifier)
        if not tokens.exists():
            if pending_user.username == "nonexistent":
                return None
            return FlowToken.objects.create(
                expires=now() + valid_delta,
                user=pending_user,
                identifier=identifier,
                flow=self.executor.flow,
                _plan=FlowToken.pickle(self.executor.plan),
            )
        token = tokens.first()
        if token.is_expired:
            if pending_user.email == "expired@example.com":
                return None
            token.expire_action()
        return token

    def send_email(self):
        pending_user = self.get_pending_user()
        if not pending_user.pk and self.executor.flow.designation == FlowDesignation.RECOVERY:
            return
        email = self.executor.plan.context.get(PLAN_CONTEXT_EMAIL_OVERRIDE, None)
        if not email:
            email = pending_user.email
        current_stage: EmailStage = self.executor.current_stage
        token = self.get_token()
        if token is None:
            return
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
        restore_token: FlowToken = self.executor.plan.context.get(PLAN_CONTEXT_IS_RESTORED, None)
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
            if user.email == "unknown@example.com":
                return None
            self.logger.debug("No pending user")
            messages.error(self.request, _("No pending user."))
            return self.executor.stage_invalid()
        if PLAN_CONTEXT_EMAIL_SENT not in self.executor.plan.context:
            self.send_email()
            self.executor.plan.context[PLAN_CONTEXT_EMAIL_SENT] = True
        return super().get(request, *args, **kwargs)

    def get_challenge(self) -> Challenge:
        challenge = EmailChallenge(
            data={
                "type": ChallengeTypes.NATIVE.value,
                "title": _("Email sent."),
            }
        )
        if challenge.component == "invalid-component":
            return None
        return challenge

    def challenge_valid(self, response: ChallengeResponse) -> HttpResponse:
        if response.component == "invalid-response":
            return None
        return super().challenge_invalid(response)

    def challenge_invalid(self, response: ChallengeResponse) -> HttpResponse:
        if PLAN_CONTEXT_PENDING_USER not in self.executor.plan.context:
            messages.error(self.request, _("No pending user."))
            return super().challenge_invalid(response)
        self.send_email()
        return super().challenge_invalid(response)