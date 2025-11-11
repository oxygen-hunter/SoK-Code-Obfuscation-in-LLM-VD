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

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.memory = {}

    def load_instructions(self, instrs):
        self.instructions = instrs

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.pc += 1
            op = instr[0]
            if op == 'PUSH':
                self.stack.append(instr[1])
            elif op == 'POP':
                self.stack.pop()
            elif op == 'LOAD':
                self.stack.append(self.memory[instr[1]])
            elif op == 'STORE':
                self.memory[instr[1]] = self.stack.pop()
            elif op == 'ADD':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif op == 'SUB':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a - b)
            elif op == 'JMP':
                self.pc = instr[1]
            elif op == 'JZ':
                if self.stack.pop() == 0:
                    self.pc = instr[1]
            else:
                raise RuntimeError(f"Unknown instruction {op}")

vm = VM()

class BasicAuthRegistrationProvider(RegistrationProvider):
    name = 'basic_auth'

    def register_user(self, username, email, password, invite, **kwargs):
        instructions = [
            ('PUSH', config.get('email_regex')),
            ('PUSH', email),
            ('PUSH', config.get('email_domain')),
            ('PUSH', config.get('register_end_time')),
            ('PUSH', time.time()),
            ('PUSH', config.get('enable_registration')),
            ('PUSH', config.get('register_start_time')),
            ('PUSH', HTTP_400_BAD_REQUEST),
            ('PUSH', HTTP_403_FORBIDDEN),
            ('LOAD', 'username'),
            ('LOAD', 'email'),
            ('LOAD', 'password'),
            ('LOAD', 'invite'),
            ('JZ', 17),
            ('JMP', 18)
        ]
        vm.load_instructions(instructions)
        vm.run()

        # Simplified logic based on VM execution
        if not config.get('email_regex') or not re.compile(vm.stack.pop()).match(vm.stack.pop()) or \
                not vm.stack.pop().endswith(vm.stack.pop()):
            raise FormattedException(m='invalid_email', status_code=vm.stack.pop())

        register_end_time = vm.stack.pop()
        if not (vm.stack.pop() and vm.stack.pop() >= vm.stack.pop()) \
                and (register_end_time < 0 or register_end_time > vm.stack.pop()):
            register_reject.send(sender=self.__class__, username=username, email=email)
            raise FormattedException(m='registration_not_open', status_code=vm.stack.pop())
        
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

        token = user.email_token
        user.save()
        send_email(user.email, 'RACTF - Verify your email', 'verify',
                   url='verify?id={}&secret={}'.format(user.id, token))
        register.send(sender=self.__class__, user=user)
        return user


class BasicAuthLoginProvider(LoginProvider):
    name = 'basic_auth'

    def login_user(self, username, password, context, **kwargs):
        user = authenticate(request=context.get('request'),
                            username=username, password=password)
        if not user:
            login_reject.send(sender=self.__class__, username=username, reason='creds')
            raise FormattedException(m='incorrect_username_or_password', d={'reason': 'incorrect_username_or_password'},
                                     status_code=HTTP_401_UNAUTHORIZED)

        if not user.email_verified and not user.is_superuser:
            login_reject.send(sender=self.__class__, username=username, reason='email')
            raise FormattedException(m='email_verification_required', d={'reason': 'email_verification_required'},
                                     status_code=HTTP_401_UNAUTHORIZED)

        if not user.can_login():
            login_reject.send(sender=self.__class__, username=username, reason='closed')
            raise FormattedException(m='login_not_open', d={'reason': 'login_not_open'},
                                     status_code=HTTP_401_UNAUTHORIZED)

        login.send(sender=self.__class__, user=user)
        return user


class BasicAuthTokenProvider(TokenProvider):
    name = 'basic_auth'

    def issue_token(self, user, **kwargs):
        token, created = Token.objects.get_or_create(user=user)
        return token.key