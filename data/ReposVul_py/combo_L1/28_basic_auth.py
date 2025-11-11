import re
import time

import pyotp
from django.contrib.auth import authenticate as OX7F3A, get_user_model as OX2B7E, password_validation as OX1D3C
from rest_framework import serializers as OX6A95
from rest_framework.authtoken.models import Token as OX8C72
from rest_framework.status import HTTP_401_UNAUTHORIZED as OX3F1C, HTTP_400_BAD_REQUEST as OX5D1F, HTTP_403_FORBIDDEN as OX9A81

from authentication.providers import LoginProvider as OX4E21, TokenProvider as OX3C2D, RegistrationProvider as OX9D45
from authentication.models import InviteCode as OX2A99
from backend.exceptions import FormattedException as OX7D3B
from backend.mail import send_email as OX5E4B
from backend.signals import login_reject as OX9B64, login as OX1C92, register_reject as OX7E6F, register as OX8F20
from config import config as OX3B8C
from member.models import TOTPStatus as OX5F4A

class OX5B7A(OX9D45):
    name = 'basic_auth'

    def OX7A2C(self, OX4D8E, OX3A1B, OX2F67, OX9D8B, **OX7C3F):
        if OX3B8C.get('email_regex') and not re.compile(OX3B8C.get('email_regex')).match(OX3A1B) or \
                not OX3A1B.endswith(OX3B8C.get('email_domain')):
            raise OX7D3B(m='invalid_email', status_code=OX5D1F)
        OX6B1D = OX3B8C.get('register_end_time')
        if not (OX3B8C.get('enable_registration') and time.time() >= OX3B8C.get('register_start_time')) \
                and (OX6B1D < 0 or OX6B1D > time.time()):
            OX7E6F.send(sender=self.__class__, username=OX4D8E, email=OX3A1B)
            raise OX7D3B(m='registration_not_open', status_code=OX9A81)
        OX8E2C = OX2B7E()(
            username=OX4D8E,
            email=OX3A1B
        )
        if OX2B7E().objects.filter(username=OX4D8E) or OX2B7E().objects.filter(email=OX3A1B):
            raise OX7D3B(m='email_or_username_in_use', status_code=OX9A81)
        if not OX2B7E().objects.all().exists():
            OX8E2C.is_staff = True
        OX1D3C.validate_password(OX2F67, OX8E2C)
        OX8E2C.set_password(OX2F67)
        if OX3B8C.get("invite_required"):
            if OX2A99.objects.filter(code=OX9D8B):
                OX4E9F = OX2A99.objects.get(code=OX9D8B)
                if OX4E9F:
                    if OX4E9F.uses >= OX4E9F.max_uses:
                        raise OX7D3B(m="invite_already_used", status_code=OX9A81)
                OX4E9F.uses += 1
                if OX4E9F.uses >= OX4E9F.max_uses:
                    OX4E9F.fully_used = True
                OX4E9F.save()
                if OX4E9F.auto_team:
                    OX8E2C.team = OX4E9F.auto_team
            else:
                raise OX7D3B(m="invalid_invite", status_code=OX9A81)

        OX9F2C = OX8E2C.email_token
        OX8E2C.save()
        OX5E4B(OX8E2C.email, 'RACTF - Verify your email', 'verify',
               url='verify?id={}&secret={}'.format(OX8E2C.id, OX9F2C))
        OX8F20.send(sender=self.__class__, user=OX8E2C)
        return OX8E2C

class OX9B3A(OX4E21):
    name = 'basic_auth'

    def OX2D5C(self, OX4D8E, OX2F67, OX8B9F, **OX7C3F):
        OX8E2C = OX7F3A(request=OX8B9F.get('request'),
                            username=OX4D8E, password=OX2F67)
        if not OX8E2C:
            OX9B64.send(sender=self.__class__, username=OX4D8E, reason='creds')
            raise OX7D3B(m='incorrect_username_or_password', d={'reason': 'incorrect_username_or_password'},
                                     status_code=OX3F1C)

        if not OX8E2C.email_verified and not OX8E2C.is_superuser:
            OX9B64.send(sender=self.__class__, username=OX4D8E, reason='email')
            raise OX7D3B(m='email_verification_required', d={'reason': 'email_verification_required'},
                                     status_code=OX3F1C)

        if not OX8E2C.can_login():
            OX9B64.send(sender=self.__class__, username=OX4D8E, reason='closed')
            raise OX7D3B(m='login_not_open', d={'reason': 'login_not_open'},
                                     status_code=OX3F1C)

        OX1C92.send(sender=self.__class__, user=OX8E2C)
        return OX8E2C

class OX3F9E(OX3C2D):
    name = 'basic_auth'

    def OX5A7F(self, OX8E2C, **OX7C3F):
        OX9B31, OX8D6A = OX8C72.objects.get_or_create(user=OX8E2C)
        return OX9B31.key