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
        def get_email_regex():
            return config.get('email_regex')
        
        def get_email_domain():
            return config.get('email_domain')
        
        def get_register_end_time():
            return config.get('register_end_time')
        
        def get_enable_registration():
            return config.get('enable_registration')
        
        def get_register_start_time():
            return config.get('register_start_time')
        
        def get_invite_required():
            return config.get("invite_required")
        
        if get_email_regex() and not re.compile(get_email_regex()).match(email) or \
                not email.endswith(get_email_domain()):
            raise FormattedException(m='invalid_email', status_code=HTTP_400_BAD_REQUEST)
        register_end_time = get_register_end_time()
        if not (get_enable_registration() and time.time() >= get_register_start_time()) \
                and (register_end_time < 0 or register_end_time > time.time()):
            register_reject.send(sender=self.__class__, username=username, email=email)
            raise FormattedException(m='registration_not_open', status_code=HTTP_403_FORBIDDEN)
        user = get_user_model()(
            username=username,
            email=email
        )
        if get_user_model().objects.filter(username=username) or get_user_model().objects.filter(email=email):
            raise FormattedException(m='email_or_username_in_use', status_code=HTTP_403_FORBIDDEN)
        if not get_user_model().objects.all().exists():
            user.is_staff = True
        password_validation.validate_password(password, user)
        user.set_password(password)
        if get_invite_required():
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

        token = user.email_token
        user.save()
        send_email(user.email, 'RACTF - Verify your email', 'verify',
                   url='verify?id={}&secret={}'.format(user.id, token))
        register.send(sender=self.__class__, user=user)
        return user


class BasicAuthLoginProvider(LoginProvider):
    name = 'basic_auth'

    def login_user(self, username, password, context, **kwargs):
        def get_request():
            return context.get('request')
        
        def get_reason_creds():
            return 'creds'
        
        def get_reason_email():
            return 'email'
        
        def get_reason_closed():
            return 'closed'
        
        user = authenticate(request=get_request(),
                            username=username, password=password)
        if not user:
            login_reject.send(sender=self.__class__, username=username, reason=get_reason_creds())
            raise FormattedException(m='incorrect_username_or_password', d={'reason': 'incorrect_username_or_password'},
                                     status_code=HTTP_401_UNAUTHORIZED)

        if not user.email_verified and not user.is_superuser:
            login_reject.send(sender=self.__class__, username=username, reason=get_reason_email())
            raise FormattedException(m='email_verification_required', d={'reason': 'email_verification_required'},
                                     status_code=HTTP_401_UNAUTHORIZED)

        if not user.can_login():
            login_reject.send(sender=self.__class__, username=username, reason=get_reason_closed())
            raise FormattedException(m='login_not_open', d={'reason': 'login_not_open'},
                                     status_code=HTTP_401_UNAUTHORIZED)

        login.send(sender=self.__class__, user=user)
        return user


class BasicAuthTokenProvider(TokenProvider):
    name = 'basic_auth'

    def issue_token(self, user, **kwargs):
        return self._create_token(user)
    
    def _create_token(self, user):
        token, created = Token.objects.get_or_create(user=user)
        return token.key