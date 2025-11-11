from functools import partial

import django_otp
from django.conf import settings
from django.contrib.auth.views import redirect_to_login
from django.urls import NoReverseMatch, reverse
from django.utils.functional import SimpleLazyObject
from django_otp.middleware import OTPMiddleware as _OTPMiddleware


class VerifyUserMiddleware(_OTPMiddleware):
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

    def __call__(self, request):
        dispatcher = 0
        response = None
        while dispatcher is not None:
            if dispatcher == 0:
                if hasattr(self, 'process_request'):
                    response = self.process_request(request)
                dispatcher = 1
            elif dispatcher == 1:
                if not response:
                    response = self.get_response(request)
                dispatcher = 2
            elif dispatcher == 2:
                if hasattr(self, 'process_response'):
                    response = self.process_response(request, response)
                dispatcher = None
        return response

    def process_request(self, request):
        dispatcher = 0
        while dispatcher is not None:
            if dispatcher == 0:
                if request.user:
                    request.user = SimpleLazyObject(partial(self._verify_user, request, request.user))
                dispatcher = 1
            elif dispatcher == 1:
                user = request.user
                if self._require_verified_user(request):
                    user_has_device = django_otp.user_has_device(user, confirmed=True)
                    if user_has_device and not user.is_verified():
                        return redirect_to_login(
                            request.get_full_path(), login_url=reverse("wagtail_2fa_auth")
                        )
                    elif not user_has_device and settings.WAGTAIL_2FA_REQUIRED:
                        return redirect_to_login(
                            request.get_full_path(), login_url=reverse("wagtail_2fa_device_new")
                        )
                dispatcher = None

    def _require_verified_user(self, request):
        dispatcher = 0
        result = None
        while dispatcher is not None:
            if dispatcher == 0:
                user = request.user
                if not settings.WAGTAIL_2FA_REQUIRED:
                    result = False
                    dispatcher = None
                else:
                    dispatcher = 1
            elif dispatcher == 1:
                if not user.is_authenticated:
                    result = False
                    dispatcher = None
                else:
                    dispatcher = 2
            elif dispatcher == 2:
                if not (
                    user.is_staff
                    or user.is_superuser
                    or user.has_perms(["wagtailadmin.access_admin"])
                ):
                    result = False
                    dispatcher = None
                else:
                    dispatcher = 3
            elif dispatcher == 3:
                user_has_device = django_otp.user_has_device(user, confirmed=True)
                if request.path in self._get_allowed_paths(user_has_device):
                    result = False
                    dispatcher = None
                else:
                    result = True
                    dispatcher = None
        return result

    def _get_allowed_paths(self, has_device):
        dispatcher = 0
        results = []
        allowed_url_names = None
        while dispatcher is not None:
            if dispatcher == 0:
                allowed_url_names = self._allowed_url_names
                if not has_device:
                    allowed_url_names = self._allowed_url_names_no_device
                dispatcher = 1
            elif dispatcher == 1:
                for route_name in allowed_url_names:
                    try:
                        results.append(settings.WAGTAIL_MOUNT_PATH + reverse(route_name))
                    except NoReverseMatch:
                        pass
                dispatcher = None
        return results