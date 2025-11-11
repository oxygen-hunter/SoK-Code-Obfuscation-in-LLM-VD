from functools import partial
import django_otp
from django.conf import settings
from django.contrib.auth.views import redirect_to_login
from django.urls import NoReverseMatch, reverse
from django.utils.functional import SimpleLazyObject
from django_otp.middleware import OTPMiddleware as _OTPMiddleware

class VerifyUserMiddleware(_OTPMiddleware):
    def __init__(self, get_response=None):
        self.instructions = []
        self.stack = []
        self.program_counter = 0
        self.get_response = get_response
        super().__init__(get_response)

    def __call__(self, request):
        self.instructions = self.compile_instructions(request)
        while self.program_counter < len(self.instructions):
            self.execute_instruction(self.instructions[self.program_counter])
        return self.stack.pop()

    def compile_instructions(self, request):
        bytecode = [
            ('PUSH', request),
            ('LOAD_USER',),
            ('JZ', 'END'),
            ('LOAD_USER',),
            ('REQUIRE_VERIFIED_USER',),
            ('JZ', 'CHECK_DEVICE'),
            ('USER_HAS_DEVICE',),
            ('JZ', 'REDIRECT_NEW_DEVICE'),
            ('IS_VERIFIED',),
            ('JZ', 'REDIRECT_AUTH'),
            ('JMP', 'END'),
            ('REDIRECT_NEW_DEVICE',),
            ('PUSH', reverse("wagtail_2fa_device_new")),
            ('REDIRECT',),
            ('JMP', 'END'),
            ('REDIRECT_AUTH',),
            ('PUSH', reverse("wagtail_2fa_auth")),
            ('REDIRECT',),
            ('CHECK_DEVICE',),
            ('PUSH', self.get_response),
            ('CALL',),
            ('END',)
        ]
        return bytecode

    def execute_instruction(self, instruction):
        op = instruction[0]
        if op == 'PUSH':
            self.stack.append(instruction[1])
        elif op == 'LOAD_USER':
            request = self.stack.pop()
            if request.user:
                request.user = SimpleLazyObject(partial(self._verify_user, request, request.user))
            self.stack.append(request.user)
        elif op == 'REQUIRE_VERIFIED_USER':
            request = self.stack.pop()
            result = self._require_verified_user(request)
            self.stack.append(result)
        elif op == 'USER_HAS_DEVICE':
            user = self.stack.pop()
            result = django_otp.user_has_device(user, confirmed=True)
            self.stack.append(result)
        elif op == 'IS_VERIFIED':
            user = self.stack.pop()
            self.stack.append(user.is_verified())
        elif op == 'REDIRECT':
            path = self.stack.pop()
            request = self.stack.pop()
            result = redirect_to_login(request.get_full_path(), login_url=path)
            self.stack.append(result)
        elif op == 'CALL':
            response = self.stack.pop()(self.stack.pop())
            self.stack.append(response)
        elif op == 'JZ':
            label = instruction[1]
            if not self.stack.pop():
                self.program_counter = self.find_label(label) - 1
        elif op == 'JMP':
            label = instruction[1]
            self.program_counter = self.find_label(label) - 1
        self.program_counter += 1

    def find_label(self, label):
        for i, ins in enumerate(self.instructions):
            if ins[0] == label:
                return i
        return -1

    def _require_verified_user(self, request):
        user = request.user
        if not settings.WAGTAIL_2FA_REQUIRED:
            return False
        if not user.is_authenticated:
            return False
        if not (user.is_staff or user.is_superuser or user.has_perms(["wagtailadmin.access_admin"])):
            return False
        user_has_device = django_otp.user_has_device(user, confirmed=True)
        if request.path in self._get_allowed_paths(user_has_device):
            return False
        return True

    def _get_allowed_paths(self, has_device):
        allowed_url_names = self._allowed_url_names if has_device else self._allowed_url_names_no_device
        results = []
        for route_name in allowed_url_names:
            try:
                results.append(settings.WAGTAIL_MOUNT_PATH + reverse(route_name))
            except NoReverseMatch:
                pass
        return results

    _allowed_url_names = [
        "wagtail_2fa_auth",
        "wagtailadmin_login",
        "wagtailadmin_logout",
    ]
    _allowed_url_names_no_device = [
        "wagtail_2fa_auth",
        "wagtail_2fa_device_list",
        "wagtail_2fa_device_new",
        "wagtail_2fa_device_qrcode",
        "wagtailadmin_login",
        "wagtailadmin_logout",
    ]