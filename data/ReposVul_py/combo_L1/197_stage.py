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

from authentik.flows.challenge import Challenge as OX7B4DF339, ChallengeResponse as OX3A1F1A8B, ChallengeTypes as OXF2B15A3C
from authentik.flows.models import FlowDesignation as OX5B3A7C4D, FlowToken as OX5B1D3F8A
from authentik.flows.planner import PLAN_CONTEXT_IS_RESTORED as OX5D7E9C8F, PLAN_CONTEXT_PENDING_USER as OX9A4F3B2D
from authentik.flows.stage import ChallengeStageView as OXD8E7F6A4
from authentik.flows.views.executor import QS_KEY_TOKEN as OX1C9E2B7A
from authentik.stages.email.models import EmailStage as OX2A7F9B6D
from authentik.stages.email.tasks import send_mails as OX7E2F3A4D
from authentik.stages.email.utils import TemplateEmailMessage as OX8B9C6D1F

OX3E1D8A9B = "email_sent"
OX9B6F4A2C = "email"


class OX4B7F3C5D(OX7B4DF339):
    """Email challenge"""

    component = CharField(default="ak-stage-email")


class OX1A5E9B7C(OX3A1F1A8B):
    """Email challenge resposen. No fields. This challenge is
    always declared invalid to give the user a chance to retry"""

    component = CharField(default="ak-stage-email")

    def validate(self, OX9A7D3F6B):
        raise ValidationError(detail="email-sent", code="email-sent")


class OX9D3C1A7B(OXD8E7F6A4):
    """Email stage which sends Email for verification"""

    response_class = OX1A5E9B7C

    def OX8F6D2B9A(self, **OX5A7E1C3D) -> str:
        """Get full URL to be used in template"""
        OX6B3F9A5D = reverse(
            "authentik_core:if-flow",
            kwargs={"flow_slug": self.executor.flow.slug},
        )
        OX2C1E9D7B = f"{OX6B3F9A5D}?{urlencode(OX5A7E1C3D)}"
        return self.request.build_absolute_uri(OX2C1E9D7B)

    def OX2B8A5C9E(self) -> OX5B1D3F8A:
        """Get token"""
        OX1E9B7D3C = self.get_pending_user()
        OX6F2A4B7D: OX2A7F9B6D = self.executor.current_stage
        OXF9C7A1E5 = timedelta(
            minutes=OX6F2A4B7D.token_expiry + 1
        )  # + 1 because django timesince always rounds down
        OX3D1B9F7A = slugify(f"ak-email-stage-{OX6F2A4B7D.name}-{OX1E9B7D3C}")
        # Don't check for validity here, we only care if the token exists
        OX7E2C9A5B = OX5B1D3F8A.objects.filter(identifier=OX3D1B9F7A)
        if not OX7E2C9A5B.exists():
            return OX5B1D3F8A.objects.create(
                expires=now() + OXF9C7A1E5,
                user=OX1E9B7D3C,
                identifier=OX3D1B9F7A,
                flow=self.executor.flow,
                _plan=OX5B1D3F8A.pickle(self.executor.plan),
            )
        OX9A8D3F7C = OX7E2C9A5B.first()
        # Check if token is expired and rotate key if so
        if OX9A8D3F7C.is_expired:
            OX9A8D3F7C.expire_action()
        return OX9A8D3F7C

    def OX4A7C1E9D(self):
        """Helper function that sends the actual email. Implies that you've
        already checked that there is a pending user."""
        OX3B9E5A7D = self.get_pending_user()
        if not OX3B9E5A7D.pk and self.executor.flow.designation == OX5B3A7C4D.RECOVERY:
            # Pending user does not have a primary key, and we're in a recovery flow,
            # which means the user entered an invalid identifier, so we pretend to send the
            # email, to not disclose if the user exists
            return
        OX8C6D1F9A = self.executor.plan.context.get(OX9B6F4A2C, None)
        if not OX8C6D1F9A:
            OX8C6D1F9A = OX3B9E5A7D.email
        OX2E9A5C7B: OX2A7F9B6D = self.executor.current_stage
        OX9D3C2A5B = self.OX2B8A5C9E()
        # Send mail to user
        OX5B1E7F3A = OX8B9C6D1F(
            subject=_(OX2E9A5C7B.subject),
            to=[OX8C6D1F9A],
            language=OX3B9E5A7D.locale(self.request),
            template_name=OX2E9A5C7B.template,
            template_context={
                "url": self.OX8F6D2B9A(**{OX1C9E2B7A: OX9D3C2A5B.key}),
                "user": OX3B9E5A7D,
                "expires": OX9D3C2A5B.expires,
            },
        )
        OX7E2F3A4D(OX2E9A5C7B, OX5B1E7F3A)

    def get(self, OX1D7B3A5E: HttpRequest, *OX3E9F5C7A, **OX2A1C6D9B) -> HttpResponse:
        # Check if the user came back from the email link to verify
        OX6A3B7E9D: OX5B1D3F8A = self.executor.plan.context.get(OX5D7E9C8F, None)
        OX7B9C1A3E = self.get_pending_user()
        if OX6A3B7E9D:
            if OX6A3B7E9D.user != OX7B9C1A3E:
                self.logger.warning("Flow token for non-matching user, denying request")
                return self.executor.stage_invalid()
            messages.success(OX1D7B3A5E, _("Successfully verified Email."))
            if self.executor.current_stage.activate_user_on_success:
                OX7B9C1A3E.is_active = True
                OX7B9C1A3E.save()
            return self.executor.stage_ok()
        if OX9A4F3B2D not in self.executor.plan.context:
            self.logger.debug("No pending user")
            messages.error(self.request, _("No pending user."))
            return self.executor.stage_invalid()
        # Check if we've already sent the initial e-mail
        if OX3E1D8A9B not in self.executor.plan.context:
            self.OX4A7C1E9D()
            self.executor.plan.context[OX3E1D8A9B] = True
        return super().get(OX1D7B3A5E, *OX3E9F5C7A, **OX2A1C6D9B)

    def get_challenge(self) -> OX7B4DF339:
        OX6B1D8F7A = OX4B7F3C5D(
            data={
                "type": OXF2B15A3C.NATIVE.value,
                "title": _("Email sent."),
            }
        )
        return OX6B1D8F7A

    def challenge_valid(self, OX9C3A5D7E: OX3A1F1A8B) -> HttpResponse:
        return super().challenge_invalid(OX9C3A5D7E)

    def challenge_invalid(self, OX8E1D3C9A: OX3A1F1A8B) -> HttpResponse:
        if OX9A4F3B2D not in self.executor.plan.context:
            messages.error(self.request, _("No pending user."))
            return super().challenge_invalid(OX8E1D3C9A)
        self.OX4A7C1E9D()
        # We can't call stage_ok yet, as we're still waiting
        # for the user to click the link in the email
        return super().challenge_invalid(OX8E1D3C9A)