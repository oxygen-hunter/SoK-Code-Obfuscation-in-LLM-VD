from typing import List, Optional, Tuple

from django.conf import settings
from django.contrib.auth.models import AnonymousUser
from django.utils.crypto import constant_time_compare
from django.utils.encoding import force_str
from rest_framework.authentication import (
    BasicAuthentication,
    SessionAuthentication,
    get_authorization_header,
)
from rest_framework.exceptions import AuthenticationFailed
from rest_framework.request import Request
from sentry_relay import UnpackError

from sentry import options
from sentry.auth.system import SystemToken, is_internal_ip
from sentry.models import ApiApplication, ApiKey, ApiToken, OrgAuthToken, ProjectKey, Relay
from sentry.relay.utils import get_header_relay_id, get_header_relay_signature
from sentry.services.hybrid_cloud.rpc import compare_signature
from sentry.utils.sdk import configure_scope
from sentry.utils.security.orgauthtoken_token import SENTRY_ORG_AUTH_TOKEN_PREFIX, hash_token


class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_program(self, program):
        self.instructions = program

    def run(self):
        while self.pc < len(self.instructions):
            instruction = self.instructions[self.pc]
            self.pc += 1
            self.execute(instruction)

    def execute(self, instruction):
        op, *args = instruction
        if op == 'PUSH':
            self.stack.append(args[0])
        elif op == 'POP':
            self.stack.pop()
        elif op == 'ADD':
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)
        elif op == 'JMP':
            self.pc = args[0]
        elif op == 'JZ':
            if self.stack.pop() == 0:
                self.pc = args[0]
        elif op == 'LOAD':
            self.stack.append(getattr(self, args[0]))
        elif op == 'STORE':
            setattr(self, args[0], self.stack.pop())
        elif op == 'CALL':
            args[0](*args[1:])
        else:
            raise RuntimeError(f"Unknown operation {op}")

def is_internal_relay(request, public_key):
    vm = VM()
    vm.load_program([
        ('LOAD', 'DEBUG'), ('PUSH', 'SENTRY_RELAY_WHITELIST_PK'), 
        ('LOAD', 'public_key'), ('CALL', lambda a, b: b in a, 2), 
        ('ADD',), ('JZ', 6), ('PUSH', True), ('JMP', 8), 
        ('PUSH', False), ('STORE', 'result')
    ])
    vm.DEBUG = settings.DEBUG
    vm.public_key = public_key
    vm.run()
    return vm.result or is_internal_ip(request)

def is_static_relay(request):
    vm = VM()
    vm.load_program([
        ('LOAD', 'relay_id'), ('LOAD', 'static_relays'), 
        ('CALL', lambda a, b: b.get(a), 2), ('STORE', 'relay_info'), 
        ('LOAD', 'relay_info'), ('CALL', lambda x: x is not None, 1), 
        ('STORE', 'result')
    ])
    vm.relay_id = get_header_relay_id(request)
    vm.static_relays = options.get("relay.static_auth")
    vm.run()
    return vm.result

def relay_from_id(request, relay_id) -> Tuple[Optional[Relay], bool]:
    vm = VM()
    vm.load_program([
        ('LOAD', 'static_relays'), ('LOAD', 'relay_id'), 
        ('CALL', lambda a, b: a.get(b), 2), ('STORE', 'relay_info'), 
        ('LOAD', 'relay_info'), ('CALL', lambda x: x is not None, 1), 
        ('JZ', 6), ('LOAD', 'relay_info'), 
        ('CALL', lambda x: Relay(relay_id=relay_id, public_key=x.get("public_key"), is_internal=x.get("internal") is True), 1), 
        ('PUSH', True), ('JMP', 14), ('LOAD', 'relay_id'), 
        ('CALL', lambda x: Relay.objects.get(relay_id=x), 1), 
        ('STORE', 'relay'), ('LOAD', 'relay'), 
        ('LOAD', 'public_key'), ('CALL', is_internal_relay, 2), 
        ('STORE', 'relay.is_internal'), ('LOAD', 'relay'), 
        ('PUSH', False), ('STORE', 'result')
    ])
    vm.static_relays = options.get("relay.static_auth")
    vm.relay_id = relay_id
    vm.public_key = request
    vm.run()
    return (vm.relay, vm.result)

class QuietBasicAuthentication(BasicAuthentication):
    def authenticate_header(self, request: Request):
        return 'xBasic realm="%s"' % self.www_authenticate_realm

class StandardAuthentication(QuietBasicAuthentication):
    token_name = None

    def accepts_auth(self, auth: "list[bytes]") -> bool:
        return auth and auth[0].lower() == self.token_name

    def authenticate(self, request: Request):
        auth = get_authorization_header(request).split()

        if not self.accepts_auth(auth):
            return None

        if len(auth) == 1:
            raise AuthenticationFailed("Invalid token header. No credentials provided.")
        elif len(auth) > 2:
            raise AuthenticationFailed("Invalid token header. Token string should not contain spaces.")

        return self.authenticate_credentials(request, force_str(auth[1]))

class RelayAuthentication(BasicAuthentication):
    def authenticate(self, request: Request):
        relay_id = get_header_relay_id(request)
        relay_sig = get_header_relay_signature(request)
        if not relay_id:
            raise AuthenticationFailed("Invalid relay ID")
        if not relay_sig:
            raise AuthenticationFailed("Missing relay signature")
        return self.authenticate_credentials(relay_id, relay_sig, request)

    def authenticate_credentials(self, relay_id, relay_sig, request):
        with configure_scope() as scope:
            scope.set_tag("relay_id", relay_id)

        relay, static = relay_from_id(request, relay_id)

        if relay is None:
            raise AuthenticationFailed("Unknown relay")

        try:
            data = relay.public_key_object.unpack(request.body, relay_sig, max_age=60 * 5)
            request.relay = relay
            request.relay_request_data = data
        except UnpackError:
            raise AuthenticationFailed("Invalid relay signature")

        return (AnonymousUser(), None)

class ApiKeyAuthentication(QuietBasicAuthentication):
    token_name = b"basic"

    def accepts_auth(self, auth: "list[bytes]") -> bool:
        return auth and auth[0].lower() == self.token_name

    def authenticate_credentials(self, userid, password, request=None):
        if password:
            return None

        try:
            key = ApiKey.objects.get_from_cache(key=userid)
        except ApiKey.DoesNotExist:
            raise AuthenticationFailed("API key is not valid")

        if not key.is_active:
            raise AuthenticationFailed("Key is disabled")

        with configure_scope() as scope:
            scope.set_tag("api_key", key.id)

        return (AnonymousUser(), key)

class SessionNoAuthTokenAuthentication(SessionAuthentication):
    def authenticate(self, request: Request):
        auth = get_authorization_header(request)
        if auth:
            return None
        return super().authenticate(request)

class ClientIdSecretAuthentication(QuietBasicAuthentication):
    def authenticate(self, request: Request):
        if not request.json_body:
            raise AuthenticationFailed("Invalid request")

        client_id = request.json_body.get("client_id")
        client_secret = request.json_body.get("client_secret")

        invalid_pair_error = AuthenticationFailed("Invalid Client ID / Secret pair")

        if not client_id or not client_secret:
            raise invalid_pair_error

        try:
            application = ApiApplication.objects.get(client_id=client_id)
        except ApiApplication.DoesNotExist:
            raise invalid_pair_error

        if not constant_time_compare(application.client_secret, client_secret):
            raise invalid_pair_error

        try:
            return (application.sentry_app.proxy_user, None)
        except Exception:
            raise invalid_pair_error

class TokenAuthentication(StandardAuthentication):
    token_name = b"bearer"

    def accepts_auth(self, auth: "list[bytes]") -> bool:
        if not super().accepts_auth(auth):
            return False

        if len(auth) != 2:
            return True

        token_str = force_str(auth[1])
        return not token_str.startswith(SENTRY_ORG_AUTH_TOKEN_PREFIX)

    def authenticate_credentials(self, request: Request, token_str):
        token = SystemToken.from_request(request, token_str)
        try:
            token = (
                token
                or ApiToken.objects.filter(token=token_str)
                .select_related("user", "application")
                .get()
            )
        except ApiToken.DoesNotExist:
            raise AuthenticationFailed("Invalid token")

        if token.is_expired():
            raise AuthenticationFailed("Token expired")

        if not token.user.is_active:
            raise AuthenticationFailed("User inactive or deleted")

        if token.application and not token.application.is_active:
            raise AuthenticationFailed("UserApplication inactive or deleted")

        with configure_scope() as scope:
            scope.set_tag("api_token_type", self.token_name)
            scope.set_tag("api_token", token.id)
            scope.set_tag("api_token_is_sentry_app", getattr(token.user, "is_sentry_app", False))

        return (token.user, token)

class OrgAuthTokenAuthentication(StandardAuthentication):
    token_name = b"bearer"

    def accepts_auth(self, auth: "list[bytes]") -> bool:
        if not super().accepts_auth(auth) or len(auth) != 2:
            return False

        token_str = force_str(auth[1])
        return token_str.startswith(SENTRY_ORG_AUTH_TOKEN_PREFIX)

    def authenticate_credentials(self, request: Request, token_str):
        token = None
        token_hashed = hash_token(token_str)

        try:
            token = OrgAuthToken.objects.filter(
                token_hashed=token_hashed, date_deactivated__isnull=True
            ).get()
        except OrgAuthToken.DoesNotExist:
            raise AuthenticationFailed("Invalid org token")

        with configure_scope() as scope:
            scope.set_tag("api_token_type", self.token_name)
            scope.set_tag("api_token", token.id)
            scope.set_tag("api_token_is_org_token", True)

        return (AnonymousUser(), token)

class DSNAuthentication(StandardAuthentication):
    token_name = b"dsn"

    def authenticate_credentials(self, request: Request, token):
        try:
            key = ProjectKey.from_dsn(token)
        except ProjectKey.DoesNotExist:
            raise AuthenticationFailed("Invalid dsn")

        if not key.is_active:
            raise AuthenticationFailed("Invalid dsn")

        with configure_scope() as scope:
            scope.set_tag("api_token_type", self.token_name)
            scope.set_tag("api_project_key", key.id)

        return (AnonymousUser(), key)

class RpcSignatureAuthentication(StandardAuthentication):
    token_name = b"rpcsignature"

    def accepts_auth(self, auth: List[bytes]) -> bool:
        if not auth or len(auth) < 2:
            return False
        return auth[0].lower() == self.token_name

    def authenticate_credentials(self, request: Request, token: str):
        if not compare_signature(request.path_info, request.body, token):
            raise AuthenticationFailed("Invalid signature")

        with configure_scope() as scope:
            scope.set_tag("rpc_auth", True)

        return (AnonymousUser(), token)