from functools import partial
import django_otp
from django.conf import settings
from django.contrib.auth.views import redirect_to_login
from django.urls import NoReverseMatch, reverse
from django.utils.functional import SimpleLazyObject
from django_otp.middleware import OTPMiddleware as _OTPMiddleware

def getAllowedUrlNames():
    return [
        "wagtail_2fa_auth",
        "wagtailadmin_login",
        "wagtailadmin_logout",
    ]

def getAllowedUrlNamesNoDevice():
    return [
        "wagtail_2fa_auth",
        "wagtail_2fa_device_list",
        "wagtail_2fa_device_new",
        "wagtail_2fa_device_qrcode",
        "wagtailadmin_login",
        "wagtailadmin_logout",
    ]

class VerifyUserMiddleware(_OTPMiddleware):
    def __call__(self, request):
        resp = None
        if hasattr(self, 'process_request'):
            resp = self.process_request(request)
        if not resp:
            resp = self.get_response(request)
        if hasattr(self, 'process_response'):
            resp = self.process_response(request, resp)
        return resp

    def process_request(self, request):
        if request.user:
            request.user = SimpleLazyObject(partial(self._verify_user, request, request.user))
        u = request.user
        if self._require_verified_user(request):
            has_device = django_otp.user_has_device(u, confirmed=True)

            if has_device and not u.is_verified():
                return redirect_to_login(
                    request.get_full_path(), login_url=reverse("wagtail_2fa_auth")
                )

            elif not has_device and settings.WAGTAIL_2FA_REQUIRED:
                return redirect_to_login(
                    request.get_full_path(), login_url=reverse("wagtail_2fa_device_new")
                )

    def _require_verified_user(self, request):
        u = request.user

        if not settings.WAGTAIL_2FA_REQUIRED:
            return False

        if not u.is_authenticated:
            return False

        if not (
            u.is_staff
            or u.is_superuser
            or u.has_perms(["wagtailadmin.access_admin"])
        ):
            return False

        has_device = django_otp.user_has_device(u, confirmed=True)
        if request.path in self._get_allowed_paths(has_device):
            return False

        return True

    def _get_allowed_paths(self, has_device):
        allowed_names = getAllowedUrlNames()
        if not has_device:
            allowed_names = getAllowedUrlNamesNoDevice()

        res = []
        for rn in allowed_names:
            try:
                res.append(settings.WAGTAIL_MOUNT_PATH + reverse(rn))
            except NoReverseMatch:
                pass
        return res