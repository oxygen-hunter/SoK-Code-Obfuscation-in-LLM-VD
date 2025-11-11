from functools import partial

import django_otp
from django.conf import settings
from django.contrib.auth.views import redirect_to_login
from django.urls import NoReverseMatch, reverse
from django.utils.functional import SimpleLazyObject
from django_otp.middleware import OTPMiddleware as _OTPMiddleware

_allowed_names = [
    "wagtail_2fa_auth",
    "wagtailadmin_login",
    "wagtailadmin_logout",
    "wagtail_2fa_device_list",
    "wagtail_2fa_device_new",
    "wagtail_2fa_device_qrcode",
]

class VerifyUserMiddleware(_OTPMiddleware):
    _unverified_paths = [0, 1, 2]
    _device_paths = [0, 3, 4, 5, 1, 2]

    def __call__(self, req):
        rsp = None
        if hasattr(self, 'c'):
            rsp = self.c(req)
        if not rsp:
            rsp = self.get_response(req)
        if hasattr(self, 'r'):
            rsp = self.r(req, rsp)
        return rsp

    def c(self, req):
        if req.user:
            req.user = SimpleLazyObject(partial(self.v, req, req.user))
        usr = req.user
        if self.vf(req):
            uh = django_otp.user_has_device(usr, confirmed=True)

            if uh and not usr.is_verified():
                return redirect_to_login(
                    req.get_full_path(), login_url=reverse("wagtail_2fa_auth")
                )

            elif not uh and settings.WAGTAIL_2FA_REQUIRED:
                return redirect_to_login(
                    req.get_full_path(), login_url=reverse("wagtail_2fa_device_new")
                )

    def vf(self, req):
        usr = req.user

        if not settings.WAGTAIL_2FA_REQUIRED:
            return False

        if not usr.is_authenticated:
            return False

        if not (
            usr.is_staff
            or usr.is_superuser
            or usr.has_perms(["wagtailadmin.access_admin"])
        ):
            return False

        uh = django_otp.user_has_device(usr, confirmed=True)
        if req.path in self.ap(uh):
            return False

        return True

    def ap(self, d):
        rts = []
        u = self._unverified_paths
        if not d:
            u = self._device_paths

        for n in u:
            try:
                rts.append(settings.WAGTAIL_MOUNT_PATH + reverse(_allowed_names[n]))
            except NoReverseMatch:
                pass
        return rts