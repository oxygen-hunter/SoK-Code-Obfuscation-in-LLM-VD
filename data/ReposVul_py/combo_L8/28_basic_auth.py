import re as _r
import time as _t
import ctypes as _ctypes

import pyotp as _po
from django.contrib.auth import authenticate, get_user_model, password_validation
from rest_framework import serializers as _srlz
from rest_framework.authtoken.models import Token as _Token
from rest_framework.status import HTTP_401_UNAUTHORIZED as _U, HTTP_400_BAD_REQUEST as _B, HTTP_403_FORBIDDEN as _F

from authentication.providers import LoginProvider as _LP, TokenProvider as _TP, RegistrationProvider as _RP
from authentication.models import InviteCode as _IC
from backend.exceptions import FormattedException as _FE
from backend.mail import send_email as _se
from backend.signals import login_reject as _lr, login as _l, register_reject as _rr, register as _r
from config import config as _cfg
from member.models import TOTPStatus as _TS

_c_code = """
#include <stdbool.h>

bool email_valid(const char* email, const char* regex, const char* domain) {
    if (!regex || !domain) return false;
    // Simulates regex and domain checks
    return strstr(email, domain) != NULL;
}

bool registration_open(bool enable_registration, double current_time, double start_time, double end_time) {
    return enable_registration && current_time >= start_time && (end_time < 0 || end_time > current_time);
}
"""
_c_lib = _ctypes.CDLL(None)
_c_lib.email_valid = _ctypes.CFUNCTYPE(_ctypes.c_bool, _ctypes.c_char_p, _ctypes.c_char_p, _ctypes.c_char_p)(_c_code.encode('utf-8'))
_c_lib.registration_open = _ctypes.CFUNCTYPE(_ctypes.c_bool, _ctypes.c_bool, _ctypes.c_double, _ctypes.c_double, _ctypes.c_double)(_c_code.encode('utf-8'))

class BasicAuthRegistrationProvider(_RP):
    name = 'basic_auth'

    def register_user(self, username, email, password, invite, **kwargs):
        if not _c_lib.email_valid(email.encode(), _cfg.get('email_regex').encode(), _cfg.get('email_domain').encode()):
            raise _FE(m='invalid_email', status_code=_B)
        
        if not _c_lib.registration_open(_cfg.get('enable_registration'), _t.time(), _cfg.get('register_start_time'), _cfg.get('register_end_time')):
            _rr.send(sender=self.__class__, username=username, email=email)
            raise _FE(m='registration_not_open', status_code=_F)
        
        user = get_user_model()(username=username, email=email)
        if get_user_model().objects.filter(username=username) or get_user_model().objects.filter(email=email):
            raise _FE(m='email_or_username_in_use', status_code=_F)
        
        if not get_user_model().objects.all().exists():
            user.is_staff = True
        
        password_validation.validate_password(password, user)
        user.set_password(password)
        
        if _cfg.get("invite_required"):
            if _IC.objects.filter(code=invite):
                code = _IC.objects.get(code=invite)
                if code:
                    if code.uses >= code.max_uses:
                        raise _FE(m="invite_already_used", status_code=_F)
                
                code.uses += 1
                if code.uses >= code.max_uses:
                    code.fully_used = True
                code.save()
                
                if code.auto_team:
                    user.team = code.auto_team
            else:
                raise _FE(m="invalid_invite", status_code=_F)

        token = user.email_token
        user.save()
        _se(user.email, 'RACTF - Verify your email', 'verify', url='verify?id={}&secret={}'.format(user.id, token))
        _r.send(sender=self.__class__, user=user)
        return user


class BasicAuthLoginProvider(_LP):
    name = 'basic_auth'

    def login_user(self, username, password, context, **kwargs):
        user = authenticate(request=context.get('request'), username=username, password=password)
        if not user:
            _lr.send(sender=self.__class__, username=username, reason='creds')
            raise _FE(m='incorrect_username_or_password', d={'reason': 'incorrect_username_or_password'}, status_code=_U)
        
        if not user.email_verified and not user.is_superuser:
            _lr.send(sender=self.__class__, username=username, reason='email')
            raise _FE(m='email_verification_required', d={'reason': 'email_verification_required'}, status_code=_U)
        
        if not user.can_login():
            _lr.send(sender=self.__class__, username=username, reason='closed')
            raise _FE(m='login_not_open', d={'reason': 'login_not_open'}, status_code=_U)
        
        _l.send(sender=self.__class__, user=user)
        return user


class BasicAuthTokenProvider(_TP):
    name = 'basic_auth'

    def issue_token(self, user, **kwargs):
        token, created = _Token.objects.get_or_create(user=user)
        return token.key