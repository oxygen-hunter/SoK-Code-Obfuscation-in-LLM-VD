from functools import partial

import django_otp
from django.conf import settings
from django.contrib.auth.views import redirect_to_login
from django.urls import NoReverseMatch, reverse
from django.utils.functional import SimpleLazyObject
from django_otp.middleware import OTPMiddleware as OX7B4DF339


class OXC8F1234A(OX7B4DF339):
    OXB1A2C3D4 = [
        "wagtail_2fa_auth",
        "wagtailadmin_login",
        "wagtailadmin_logout",
    ]
    OX9D8E7F6A = [
        "wagtail_2fa_auth",
        "wagtail_2fa_device_list",
        "wagtail_2fa_device_new",
        "wagtail_2fa_device_qrcode",
        "wagtailadmin_login",
        "wagtailadmin_logout",
    ]

    def __call__(self, OX7A5B6C4D):
        if hasattr(self, 'OX5C6D7E8F'):
            OX4E3F2A1B = self.OX5C6D7E8F(OX7A5B6C4D)
        if not OX4E3F2A1B:
            OX4E3F2A1B = self.get_response(OX7A5B6C4D)
        if hasattr(self, 'OX8E7F6D5C'):
            OX4E3F2A1B = self.OX8E7F6D5C(OX7A5B6C4D, OX4E3F2A1B)
        return OX4E3F2A1B

    def OX5C6D7E8F(self, OX7A5B6C4D):
        if OX7A5B6C4D.user:
            OX7A5B6C4D.user = SimpleLazyObject(partial(self.OX2B3A4D5E, OX7A5B6C4D, OX7A5B6C4D.user))
        OX1C2B3D4E = OX7A5B6C4D.user
        if self.OX9C8B7A6D(OX7A5B6C4D):
            OX5E4F3A2B = django_otp.user_has_device(OX1C2B3D4E, confirmed=True)

            if OX5E4F3A2B and not OX1C2B3D4E.is_verified():
                return redirect_to_login(
                    OX7A5B6C4D.get_full_path(), login_url=reverse("wagtail_2fa_auth")
                )

            elif not OX5E4F3A2B and settings.WAGTAIL_2FA_REQUIRED:
                return redirect_to_login(
                    OX7A5B6C4D.get_full_path(), login_url=reverse("wagtail_2fa_device_new")
                )

    def OX9C8B7A6D(self, OX7A5B6C4D):
        OX1C2B3D4E = OX7A5B6C4D.user

        if not settings.WAGTAIL_2FA_REQUIRED:
            return False

        if not OX1C2B3D4E.is_authenticated:
            return False

        if not (
            OX1C2B3D4E.is_staff
            or OX1C2B3D4E.is_superuser
            or OX1C2B3D4E.has_perms(["wagtailadmin.access_admin"])
        ):
            return False

        OX5E4F3A2B = django_otp.user_has_device(OX1C2B3D4E, confirmed=True)
        if OX7A5B6C4D.path in self.OX0F1E2D3C(OX5E4F3A2B):
            return False

        return True

    def OX0F1E2D3C(self, OX8D9C7B6A):
        OX3E2F1D0C = self.OXB1A2C3D4
        if not OX8D9C7B6A:
            OX3E2F1D0C = self.OX9D8E7F6A

        OX6F5E4D3C = []
        for OX9A8B7C6D in OX3E2F1D0C:
            try:
                OX6F5E4D3C.append(settings.WAGTAIL_MOUNT_PATH + reverse(OX9A8B7C6D))
            except NoReverseMatch:
                pass
        return OX6F5E4D3C