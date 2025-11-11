import re
import time

import pyotp
from django.contrib.auth import authenticate, get_user_model, password_validation
from rest_framework import serializers
from rest_framework.authtoken.models import Token
from rest_framework.status import HTTP_401_UNAUTHORIZED, HTTP_400_BAD_REQUEST, HTTP_403_FORBIDDEN

from authentication.providers import LoginProvider, TokenProvider, RegistrationProvider
from authentication.models import InviteCode
from backend.exceptions import FormattedException
from backend.mail import send_email
from backend.signals import login_reject, login, register_reject, register
from config import config
from member.models import TOTPStatus


class BasicAuthRegistrationProvider(RegistrationProvider):
    name = 'basic_auth'

    def register_user(self, a, b, c, d, **kwargs):
        if config.get('email_regex') and not re.compile(config.get('email_regex')).match(b) or \
                not b.endswith(config.get('email_domain')):
            raise FormattedException(m='invalid_email', status_code=HTTP_400_BAD_REQUEST)
        x = config.get('register_end_time')
        if not (config.get('enable_registration') and time.time() >= config.get('register_start_time')) \
                and (x < 0 or x > time.time()):
            register_reject.send(sender=self.__class__, username=a, email=b)
            raise FormattedException(m='registration_not_open', status_code=HTTP_403_FORBIDDEN)
        e = get_user_model()(
            username=a,
            email=b
        )
        if get_user_model().objects.filter(username=a) or get_user_model().objects.filter(email=b):
            raise FormattedException(m='email_or_username_in_use', status_code=HTTP_403_FORBIDDEN)
        if not get_user_model().objects.all().exists():
            e.is_staff = True
        password_validation.validate_password(c, e)
        e.set_password(c)
        if config.get("invite_required"):
            if InviteCode.objects.filter(code=d):
                y = InviteCode.objects.get(code=d)
                if y:
                    if y.uses >= y.max_uses:
                        raise FormattedException(m="invite_already_used", status_code=HTTP_403_FORBIDDEN)
                y.uses += 1
                if y.uses >= y.max_uses:
                    y.fully_used = True
                y.save()
                if y.auto_team:
                    e.team = y.auto_team
            else:
                raise FormattedException(m="invalid_invite", status_code=HTTP_403_FORBIDDEN)

        z = e.email_token
        e.save()
        send_email(e.email, 'RACTF - Verify your email', 'verify',
                   url='verify?id={}&secret={}'.format(e.id, z))
        register.send(sender=self.__class__, user=e)
        return e


class BasicAuthLoginProvider(LoginProvider):
    name = 'basic_auth'

    def login_user(self, f, g, h, **kwargs):
        i = authenticate(request=h.get('request'),
                            username=f, password=g)
        if not i:
            login_reject.send(sender=self.__class__, username=f, reason='creds')
            raise FormattedException(m='incorrect_username_or_password', d={'reason': 'incorrect_username_or_password'},
                                     status_code=HTTP_401_UNAUTHORIZED)

        if not i.email_verified and not i.is_superuser:
            login_reject.send(sender=self.__class__, username=f, reason='email')
            raise FormattedException(m='email_verification_required', d={'reason': 'email_verification_required'},
                                     status_code=HTTP_401_UNAUTHORIZED)

        if not i.can_login():
            login_reject.send(sender=self.__class__, username=f, reason='closed')
            raise FormattedException(m='login_not_open', d={'reason': 'login_not_open'},
                                     status_code=HTTP_401_UNAUTHORIZED)

        login.send(sender=self.__class__, user=i)
        return i


class BasicAuthTokenProvider(TokenProvider):
    name = 'basic_auth'

    def issue_token(self, j, **kwargs):
        k, l = Token.objects.get_or_create(user=j)
        return k.key