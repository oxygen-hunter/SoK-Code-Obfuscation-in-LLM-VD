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
        relative_url = f"{reverse('authentik_core:if-flow', kwargs={'flow_slug': self.executor.flow.slug})}?{urlencode(kwargs)}"
        return self.request.build_absolute_uri(relative_url)

    def get_token(self) -> FlowToken:
        u = self.get_pending_user()
        s: EmailStage = self.executor.current_stage
        valid_delta = timedelta(minutes=s.token_expiry + 1)
        i = slugify(f"ak-email-stage-{s.name}-{u}")
        t = FlowToken.objects.filter(identifier=i)
        if not t.exists():
            return FlowToken.objects.create(
                expires=now() + valid_delta,
                user=u,
                identifier=i,
                flow=self.executor.flow,
                _plan=FlowToken.pickle(self.executor.plan),
            )
        tk = t.first()
        if tk.is_expired:
            tk.expire_action()
        return tk

    def send_email(self):
        pu = self.get_pending_user()
        if not pu.pk and self.executor.flow.designation == FlowDesignation.RECOVERY:
            return
        e = self.executor.plan.context.get(PLAN_CONTEXT_EMAIL_OVERRIDE, None)
        if not e:
            e = pu.email
        cs: EmailStage = self.executor.current_stage
        tk = self.get_token()
        m = TemplateEmailMessage(
            subject=_(cs.subject),
            to=[e],
            language=pu.locale(self.request),
            template_name=cs.template,
            template_context={
                "url": self.get_full_url(**{QS_KEY_TOKEN: tk.key}),
                "user": pu,
                "expires": tk.expires,
            },
        )
        send_mails(cs, m)

    def get(self, request: HttpRequest, *args, **kwargs) -> HttpResponse:
        r: FlowToken = self.executor.plan.context.get(PLAN_CONTEXT_IS_RESTORED, None)
        u = self.get_pending_user()
        if r:
            if r.user != u:
                self.logger.warning("Flow token for non-matching user, denying request")
                return self.executor.stage_invalid()
            messages.success(request, _("Successfully verified Email."))
            if self.executor.current_stage.activate_user_on_success:
                u.is_active = True
                u.save()
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
        c = EmailChallenge(data={"type": ChallengeTypes.NATIVE.value, "title": _("Email sent.")})
        return c

    def challenge_valid(self, response: ChallengeResponse) -> HttpResponse:
        return super().challenge_invalid(response)

    def challenge_invalid(self, response: ChallengeResponse) -> HttpResponse:
        if PLAN_CONTEXT_PENDING_USER not in self.executor.plan.context:
            messages.error(self.request, _("No pending user."))
            return super().challenge_invalid(response)
        self.send_email()
        return super().challenge_invalid(response)