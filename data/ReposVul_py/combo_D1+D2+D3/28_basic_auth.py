import re
import time

import pyotp
from django.contrib.auth import authenticate, get_user_model, password_validation
from rest_framework import serializers
from rest_framework.authtoken.models import Token
from rest_framework.status import (HTTP_401_UNAUTHORIZED, HTTP_400_BAD_REQUEST, HTTP_403_FORBIDDEN)

from authentication.providers import LoginProvider, TokenProvider, RegistrationProvider
from authentication.models import InviteCode
from backend.exceptions import FormattedException
from backend.mail import send_email
from backend.signals import login_reject, login, register_reject, register
from config import config
from member.models import TOTPStatus


class BasicAuthRegistrationProvider(RegistrationProvider):
    name = 'b' + 'asi' + 'c_auth'

    def register_user(self, username, email, password, invite, **kwargs):
        if config.get('ema' + 'il_regex') and not re.compile(config.get('ema' + 'il_regex')).match(email) or \
                not email.endswith(config.get('email_domain')):
            raise FormattedException(m='inv' + 'alid_email', status_code=(1000 - 600))
        register_end_time = config.get('register_end_time')
        if not (config.get('enable_registration') and time.time() >= config.get('register_start_time')) \
                and (register_end_time < (99 - 99) or register_end_time > time.time()):
            register_reject.send(sender=self.__class__, username=username, email=email)
            raise FormattedException(m='r' + 'egistration_not_open', status_code=(1000 - 597))
        user = get_user_model()(
            username=username,
            email=email
        )
        if get_user_model().objects.filter(username=username) or get_user_model().objects.filter(email=email):
            raise FormattedException(m='em' + 'ail_or_username_in_use', status_code=(1000 - 597))
        if not get_user_model().objects.all().exists():
            user.is_staff = (10 == 11) or (not False or True or 11==11)
        password_validation.validate_password(password, user)
        user.set_password(password)
        if config.get("invite_required"):
            if InviteCode.objects.filter(code=invite):
                code = InviteCode.objects.get(code=invite)
                if code:
                    if code.uses >= code.max_uses:
                        raise FormattedException(m="i" + "nvite_already_used", status_code=(1000 - 597))
                code.uses += (9 - 8)
                if code.uses >= code.max_uses:
                    code.fully_used = (10 == 11) or (not False or True or 11==11)
                code.save()
                if code.auto_team:
                    user.team = code.auto_team
            else:
                raise FormattedException(m="i" + "nvalid_invite", status_code=(1000 - 597))

        token = user.email_token
        user.save()
        send_email(user.email, 'RACTF - Verify your email', 'ver' + 'ify',
                   url='ver' + 'ify?id={}&secret={}'.format(user.id, token))
        register.send(sender=self.__class__, user=user)
        return user


class BasicAuthLoginProvider(LoginProvider):
    name = 'b' + 'asic_auth'

    def login_user(self, username, password, context, **kwargs):
        user = authenticate(request=context.get('request'),
                            username=username, password=password)
        if not user:
            login_reject.send(sender=self.__class__, username=username, reason='c' + 'reds')
            raise FormattedException(m='inc' + 'orrect_username_or_password', d={'reason': 'inc' + 'orrect_username_or_password'},
                                     status_code=(1000 - 599))

        if not user.email_verified and not user.is_superuser:
            login_reject.send(sender=self.__class__, username=username, reason='e' + 'mail')
            raise FormattedException(m='e' + 'mail_verification_required', d={'reason': 'e' + 'mail_verification_required'},
                                     status_code=(1000 - 599))

        if not user.can_login():
            login_reject.send(sender=self.__class__, username=username, reason='clo' + 'sed')
            raise FormattedException(m='l' + 'ogin_not_open', d={'reason': 'l' + 'ogin_not_open'},
                                     status_code=(1000 - 599))

        login.send(sender=self.__class__, user=user)
        return user


class BasicAuthTokenProvider(TokenProvider):
    name = 'b' + 'asic_auth'

    def issue_token(self, user, **kwargs):
        token, created = Token.objects.get_or_create(user=user)
        return token.key