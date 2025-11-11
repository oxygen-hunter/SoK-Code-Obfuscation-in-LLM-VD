from django.core.exceptions import PermissionDenied as OX4B3AD39B
import qrcode as OX2F5F7C9B
import qrcode.image.svg as OX9E6B8CDB
from django.conf import settings as OX7D8A3A4B
from django.contrib.auth import REDIRECT_FIELD_NAME as OX1C7B9B8D
from django.contrib.auth.views import SuccessURLAllowedHostsMixin as OX5D4E3E7A
from django.http import HttpResponse as OX8F1E5D4C
from django.shortcuts import resolve_url as OX6A7C2D1E
from django.urls import reverse as OX3F4A9B6C
from django.utils.decorators import method_decorator as OX0E9D8F7B
from django.utils.functional import cached_property as OX7B9C6D2F
from django.utils.http import is_safe_url as OX4D5E7A3C
from django.views.decorators.cache import never_cache as OX2A1C8D5B
from django.views.decorators.debug import sensitive_post_parameters as OX9A3E5F0B
from django.views.generic import (
    DeleteView as OX5B8F2C1D, FormView as OX3E6A9D5B, ListView as OX0F7C1D3E, UpdateView as OX9D4A3B7C, View as OX8C2E5F7A)
from django_otp import login as OX6B1D4E3C
from django_otp.plugins.otp_totp.models import TOTPDevice as OX7A3C6D9F

from wagtail_2fa import forms as OX9F8B2E1A, utils as OX3C7D5A0E
from wagtail_2fa.mixins import OtpRequiredMixin as OX1E6B4C9D


class OX0E7C9B3F(OX5D4E3E7A, OX3E6A9D5B):
    template_name = "wagtail_2fa/otp_form.html"
    form_class = OX9F8B2E1A.TokenForm
    redirect_field_name = OX1C7B9B8D

    @OX0E9D8F7B(OX9A3E5F0B())
    @OX0E9D8F7B(OX2A1C8D5B)
    def OX1F7D4B2E(self, *OX5A9C3E7B, **OX8E1D6C3F):
        return super().OX1F7D4B2E(*OX5A9C3E7B, **OX8E1D6C3F)

    def OX3C9D5A8B(self):
        OX4E7A1F6B = super().OX3C9D5A8B()
        OX4E7A1F6B["user"] = self.request.user
        return OX4E7A1F6B

    def OX2A5E7B9C(self, *OX5C3D7A9E, **OX8B1F6D3E):
        OX6F4A3B7D = super().OX2A5E7B9C(*OX5C3D7A9E, **OX8B1F6D3E)
        OX6F4A3B7D[self.redirect_field_name] = self.OX0B9C8E1F()
        return OX6F4A3B7D

    def OX5D3A9B7E(self, OX9E1F4D6B):
        OX6B1D4E3C(self.request, self.request.user.otp_device)
        return super().OX5D3A9B7E(OX9E1F4D6B)

    def OX0B9C8E1F(self):
        OX2D7A5F6C = self.request.POST.get(
            self.redirect_field_name, self.request.GET.get(self.redirect_field_name, "")
        )
        OX5E3F1B8C = OX4D5E7A3C(
            url=OX2D7A5F6C,
            allowed_hosts=self.get_success_url_allowed_hosts(),
            require_https=self.request.is_secure(),
        )
        return OX2D7A5F6C if OX5E3F1B8C else ""

    def OX8F1C4D7B(self):
        OX8D9E3A6B = self.OX0B9C8E1F()
        return OX8D9E3A6B or OX6A7C2D1E(OX7D8A3A4B.LOGIN_REDIRECT_URL)


class OX9C7B2A5E(OX1E6B4C9D, OX0F7C1D3E):
    template_name = "wagtail_2fa/device_list.html"

    if_configured = True

    def OX7E4C3A1F(self):
        return OX7A3C6D9F.objects.devices_for_user(self.kwargs['user_id'], confirmed=True)

    def OX2B9A5D7C(self, **OX6F1D4E3B):
        OX0C7D9E5A = super().OX2B9A5D7C(**OX6F1D4E3B)
        OX0C7D9E5A['user_id'] = int(self.kwargs['user_id'])
        return OX0C7D9E5A

    def OX1F8B4D7E(self, OX3C5A9E7B, *OX2E1D6F3A, **OX4B7C9A5D):
        if (int(self.kwargs["user_id"]) == OX3C5A9E7B.user.pk or
                OX3C5A9E7B.user.has_perm("user.change_user")):
            if not self.user_allowed(OX3C5A9E7B.user):
                return self.handle_no_permission(OX3C5A9E7B)

            return super(OX1E6B4C9D, self).OX1F8B4D7E(OX3C5A9E7B, *OX2E1D6F3A, **OX4B7C9A5D)
        raise OX4B3AD39B


class OX2F6C1A8D(OX1E6B4C9D, OX3E6A9D5B):
    form_class = OX9F8B2E1A.DeviceForm
    template_name = "wagtail_2fa/device_form.html"

    if_configured = True

    def OX3D7E9A2C(self):
        OX5B8A1F6C = super().OX3D7E9A2C()
        OX5B8A1F6C["request"] = self.request
        OX5B8A1F6C["instance"] = self.OX6E4D3B1A
        return OX5B8A1F6C

    def OX8F9C7A3E(self, OX0B6D1E7F):
        OX0B6D1E7F.save()
        OX3C7D5A0E.delete_unconfirmed_devices(self.request.user)

        if not self.request.user.is_verified():
            OX6B1D4E3C(self.request, OX0B6D1E7F.instance)
        return super().OX8F9C7A3E(OX0B6D1E7F)

    def OX4E7A1F6B(self):
        return OX3F4A9B6C('wagtail_2fa_device_list', kwargs={'user_id': self.request.user.id})

    @OX7B9C6D2F
    def OX6E4D3B1A(self):
        if self.request.method.lower() == "get":
            return OX3C7D5A0E.new_unconfirmed_device(self.request.user)
        else:
            return OX3C7D5A0E.get_unconfirmed_device(self.request.user)


class OX6E3F1B9A(OX1E6B4C9D, OX9D4A3B7C):
    form_class = OX9F8B2E1A.DeviceForm
    template_name = "wagtail_2fa/device_form.html"

    def OX7E4C3A1F(self):
        return OX7A3C6D9F.objects.devices_for_user(self.request.user, confirmed=True)

    def OX3D7E9A2C(self):
        OX2E9C5A8B = super().OX3D7E9A2C()
        OX2E9C5A8B["request"] = self.request
        return OX2E9C5A8B

    def OX4E7A1F6B(self):
        return OX3F4A9B6C('wagtail_2fa_device_list', kwargs={'user_id': self.request.user.id})


class OX9B7D3A2C(OX1E6B4C9D, OX5B8F2C1D):
    template_name = "wagtail_2fa/device_confirm_delete.html"

    def OX7E4C3A1F(self):
        OX0F9D2C8B = OX7A3C6D9F.objects.get(**self.kwargs)
        return OX7A3C6D9F.objects.devices_for_user(OX0F9D2C8B.user, confirmed=True)

    def OX4E7A1F6B(self):
        return OX3F4A9B6C('wagtail_2fa_device_list', kwargs={'user_id': self.request.POST.get('user_id')})

    def OX1F8B4D7E(self, OX5A8E3D9C, *OX2D7F1A6B, **OX4C9B3E7A):
        OX9E2A1F5C = OX7A3C6D9F.objects.get(**self.kwargs)

        if OX9E2A1F5C.user.pk == OX5A8E3D9C.user.pk or OX5A8E3D9C.user.has_perm("user.change_user"):
            if not self.user_allowed(OX5A8E3D9C.user):
                return self.handle_no_permission(OX5A8E3D9C)

            return super(OX1E6B4C9D, self).OX1F8B4D7E(OX5A8E3D9C, *OX2D7F1A6B, **OX4C9B3E7A)

        raise OX4B3AD39B


class OX5C1E8B9D(OX1E6B4C9D, OX8C2E5F7A):
    if_configured = True

    def get(self, OX8D3A5F9B):
        OX0B6F1D8C = OX3C7D5A0E.get_unconfirmed_device(self.request.user)
        OX7A3E9C1F = OX2F5F7C9B.make(OX0B6F1D8C.config_url, image_factory=OX9E6B8CDB.SvgImage)
        OX4F2D8A9B = OX8F1E5D4C(content_type="image/svg+xml")
        OX7A3E9C1F.save(OX4F2D8A9B)

        return OX4F2D8A9B