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

    def register_user(self, username, email, password, invite, **kwargs):
        def validate_email():
            if config.get('email_regex') and not re.compile(config.get('email_regex')).match(email) or \
                    not email.endswith(config.get('email_domain')):
                raise FormattedException(m='invalid_email', status_code=HTTP_400_BAD_REQUEST)

        def check_registration_open():
            register_end_time = config.get('register_end_time')
            if not (config.get('enable_registration') and time.time() >= config.get('register_start_time')) \
                    and (register_end_time < 0 or register_end_time > time.time()):
                register_reject.send(sender=self.__class__, username=username, email=email)
                raise FormattedException(m='registration_not_open', status_code=HTTP_403_FORBIDDEN)

        def create_user():
            user = get_user_model()(username=username, email=email)
            if get_user_model().objects.filter(username=username) or get_user_model().objects.filter(email=email):
                raise FormattedException(m='email_or_username_in_use', status_code=HTTP_403_FORBIDDEN)
            if not get_user_model().objects.all().exists():
                user.is_staff = True
            password_validation.validate_password(password, user)
            user.set_password(password)
            return user

        def validate_invite_code(user):
            if config.get("invite_required"):
                if InviteCode.objects.filter(code=invite):
                    code = InviteCode.objects.get(code=invite)
                    if code:
                        if code.uses >= code.max_uses:
                            raise FormattedException(m="invite_already_used", status_code=HTTP_403_FORBIDDEN)
                    code.uses += 1
                    if code.uses >= code.max_uses:
                        code.fully_used = True
                    code.save()
                    if code.auto_team:
                        user.team = code.auto_team
                else:
                    raise FormattedException(m="invalid_invite", status_code=HTTP_403_FORBIDDEN)

        def finalize_registration(user):
            token = user.email_token
            user.save()
            send_email(user.email, 'RACTF - Verify your email', 'verify',
                       url='verify?id={}&secret={}'.format(user.id, token))
            register.send(sender=self.__class__, user=user)
            return user

        validate_email()
        check_registration_open()
        user = create_user()
        validate_invite_code(user)
        return finalize_registration(user)


class BasicAuthLoginProvider(LoginProvider):
    name = 'basic_auth'

    def login_user(self, username, password, context, **kwargs):
        def authenticate_user():
            return authenticate(request=context.get('request'), username=username, password=password)

        def validate_user(user):
            if not user:
                login_reject.send(sender=self.__class__, username=username, reason='creds')
                raise FormattedException(m='incorrect_username_or_password', d={'reason': 'incorrect_username_or_password'},
                                         status_code=HTTP_401_UNAUTHORIZED)

        def check_email_verification(user):
            if not user.email_verified and not user.is_superuser:
                login_reject.send(sender=self.__class__, username=username, reason='email')
                raise FormattedException(m='email_verification_required', d={'reason': 'email_verification_required'},
                                         status_code=HTTP_401_UNAUTHORIZED)

        def check_login_open(user):
            if not user.can_login():
                login_reject.send(sender=self.__class__, username=username, reason='closed')
                raise FormattedException(m='login_not_open', d={'reason': 'login_not_open'},
                                         status_code=HTTP_401_UNAUTHORIZED)

        user = authenticate_user()
        validate_user(user)
        check_email_verification(user)
        check_login_open(user)
        login.send(sender=self.__class__, user=user)
        return user

class BasicAuthTokenProvider(TokenProvider):
    name = 'basic_auth'

    def issue_token(self, user, **kwargs):
        token, created = Token.objects.get_or_create(user=user)
        return token.key