from datetime import timedelta
from django.utils.timezone import now
from django.conf import settings
from django.conf.urls import url
from oauthlib import oauth2
from oauth2_provider import views
from awx.main.models import RefreshToken
from awx.api.views import ApiOAuthAuthorizationRootView

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []

    def load_program(self, instructions):
        self.program = instructions

    def run(self):
        while self.pc < len(self.program):
            opcode, *args = self.program[self.pc]
            getattr(self, opcode)(*args)
            self.pc += 1

    def PUSH(self, value):
        self.stack.append(value)

    def POP(self):
        return self.stack.pop()

    def ADD(self):
        b = self.POP()
        a = self.POP()
        self.PUSH(a + b)

    def SUB(self):
        b = self.POP()
        a = self.POP()
        self.PUSH(a - b)

    def JMP(self, target):
        self.pc = target - 1

    def JZ(self, target):
        if self.POP() == 0:
            self.pc = target - 1

    def LOAD(self, var):
        self.PUSH(var)

    def STORE(self, var):
        var = self.POP()

    def RETURN(self, value):
        self.pc = len(self.program)

class TokenView(views.TokenView):
    def create_token_response(self, request):
        vm = VM()
        instructions = [
            ('LOAD', request.POST.get('grant_type')),
            ('PUSH', 'refresh_token'),
            ('SUB',),
            ('JZ', 5),
            ('LOAD', 'refresh_token' in request.POST),
            ('JZ', 16),
            ('LOAD', RefreshToken.objects.filter(token=request.POST['refresh_token']).first()),
            ('STORE', 'refresh_token'),
            ('LOAD', 'refresh_token'),
            ('JZ', 16),
            ('LOAD', settings.OAUTH2_PROVIDER.get('REFRESH_TOKEN_EXPIRE_SECONDS', 0)),
            ('STORE', 'expire_seconds'),
            ('LOAD', 'refresh_token.created'),
            ('LOAD', timedelta(seconds='expire_seconds')),
            ('ADD',),
            ('LOAD', now()),
            ('SUB',),
            ('JZ', 16),
            ('RETURN', (request.build_absolute_uri(), {}, 'The refresh token has expired.', '403')),
            ('LOAD', super(TokenView, self).create_token_response(request)),
            ('RETURN',),
        ]
        vm.load_program(instructions)
        result = vm.run()
        return result

urls = [
    url(r'^$', ApiOAuthAuthorizationRootView.as_view(), name='oauth_authorization_root_view'),
    url(r"^authorize/$", views.AuthorizationView.as_view(), name="authorize"),
    url(r"^token/$", TokenView.as_view(), name="token"),
    url(r"^revoke_token/$", views.RevokeTokenView.as_view(), name="revoke-token"),
]

__all__ = ['urls']