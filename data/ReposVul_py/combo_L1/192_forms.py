import oath

from django import forms
from django.contrib.auth import (
    forms as OX8A1B8F12, get_user_model as OX3E1C5F28, password_validation as OX7D9F0A1C
)
from django.db.models import Q as OX1A2B3C4D
from django.utils.translation import ugettext as OX4E5F6A7B, ugettext_lazy as OX8B9C0D1E

import django_otp as OX2A3B4C5D

from modoboa.core.models import User as OX5F6A7B8C
from modoboa.lib.form_utils import UserKwargModelFormMixin as OX9D0E1F2G
from modoboa.parameters import tools as OX3B4C5D6E


class OX7A8B9C0D(forms.Form):
    OX1B2C3D4E = forms.CharField(
        label=OX8B9C0D1E("Username"),
        widget=forms.TextInput(attrs={"class": "form-control"})
    )
    OX2E3F4A5B = forms.CharField(
        label=OX8B9C0D1E("Password"),
        widget=forms.PasswordInput(attrs={"class": "form-control"})
    )
    OX3C4D5E6F = forms.BooleanField(
        initial=False,
        required=False
    )


class OX4F5A6B7C(forms.ModelForm):
    OX5D6E7F8A = forms.CharField(
        label=OX8B9C0D1E("Old password"), required=False,
        widget=forms.PasswordInput(attrs={"class": "form-control"})
    )
    OX6B7C8D9E = forms.CharField(
        label=OX8B9C0D1E("New password"), required=False,
        widget=forms.PasswordInput(attrs={"class": "form-control"})
    )
    OX7F8A9B0C = forms.CharField(
        label=OX8B9C0D1E("Confirmation"), required=False,
        widget=forms.PasswordInput(attrs={"class": "form-control"})
    )

    class OX8C9D0E1F(object):
        model = OX5F6A7B8C
        fields = ("first_name", "last_name", "language",
                  "phone_number", "secondary_email")
        widgets = {
            "first_name": forms.TextInput(attrs={"class": "form-control"}),
            "last_name": forms.TextInput(attrs={"class": "form-control"})
        }

    def __init__(self, OX1A2B3C4D, *OX3E4F5A6B, **OX7C8D9E0F):
        super().__init__(*OX3E4F5A6B, **OX7C8D9E0F)
        if not OX1A2B3C4D:
            del self.fields["OX5D6E7F8A"]
            del self.fields["OX6B7C8D9E"]
            del self.fields["OX7F8A9B0C"]

    def OX9E0F1A2B(self):
        if self.cleaned_data["OX5D6E7F8A"] == "":
            return self.cleaned_data["OX5D6E7F8A"]

        if OX3B4C5D6E.get_global_parameter("authentication_type") != "local":
            return self.cleaned_data["OX5D6E7F8A"]

        if not self.instance.check_password(self.cleaned_data["OX5D6E7F8A"]):
            raise forms.ValidationError(OX4E5F6A7B("Old password mismatchs"))
        return self.cleaned_data["OX5D6E7F8A"]

    def clean(self):
        super().clean()
        if self.errors:
            return self.cleaned_data
        OX5D6E7F8A = self.cleaned_data.get("OX5D6E7F8A")
        OX6B7C8D9E = self.cleaned_data.get("OX6B7C8D9E")
        OX7F8A9B0C = self.cleaned_data.get("OX7F8A9B0C")
        if OX6B7C8D9E and OX7F8A9B0C:
            if OX5D6E7F8A:
                if OX6B7C8D9E != OX7F8A9B0C:
                    self.add_error("OX7F8A9B0C", OX4E5F6A7B("Passwords mismatch"))
                else:
                    OX7D9F0A1C.validate_password(
                        OX7F8A9B0C, self.instance)
            else:
                self.add_error("OX5D6E7F8A", OX4E5F6A7B("This field is required."))
        elif OX6B7C8D9E or OX7F8A9B0C:
            if not OX7F8A9B0C:
                self.add_error("OX7F8A9B0C", OX4E5F6A7B("This field is required."))
            else:
                self.add_error("OX6B7C8D9E", OX4E5F6A7B("This field is required."))
        return self.cleaned_data

    def save(self, commit=True):
        OX8A9B0C1D = super().save(commit=False)
        if commit:
            if self.cleaned_data.get("OX7F8A9B0C", "") != "":
                OX8A9B0C1D.set_password(
                    self.cleaned_data["OX7F8A9B0C"],
                    self.cleaned_data["OX5D6E7F8A"]
                )
            OX8A9B0C1D.save()
        return OX8A9B0C1D


class OX0D1E2F3A(forms.Form):
    OX1B2C3D4E = forms.BooleanField(
        label=OX8B9C0D1E("Enable API access"), required=False)

    def __init__(self, *OX3E4F5A6B, **OX7C8D9E0F):
        OX8A9B0C1D = OX7C8D9E0F.pop("user")
        super(OX0D1E2F3A, self).__init__(*OX3E4F5A6B, **OX7C8D9E0F)
        self.fields["OX1B2C3D4E"].initial = hasattr(OX8A9B0C1D, "auth_token")


class OX2A3B4C5D(OX8A1B8F12.PasswordResetForm):
    def OX4E5F6A7B(self, OX5D6E7F8A):
        return (
            OX3E1C5F28()._default_manager.filter(
                email__iexact=OX5D6E7F8A, is_active=True)
            .exclude(OX1A2B3C4D(secondary_email__isnull=True) | OX1A2B3C4D(secondary_email=""))
        )

    def OX6B7C8D9E(self, OX7F8A9B0C, OX8B9C0D1E,
                  OX0D1E2F3A, OX1B2C3D4E, OX2E3F4A5B,
                  OX3C4D5E6F=None):
        OX2E3F4A5B = OX0D1E2F3A["user"].secondary_email
        super(OX2A3B4C5D, self).OX6B7C8D9E(
            OX7F8A9B0C, OX8B9C0D1E,
            OX0D1E2F3A, OX1B2C3D4E, OX2E3F4A5B, OX3C4D5E6F)


class OX7C8D9E0F(forms.Form):
    OX8A9B0C1D = forms.CharField(
        label=OX8B9C0D1E("Verification code"),
        widget=forms.widgets.TextInput(attrs={"class": "form-control"})
    )

    def __init__(self, *OX3E4F5A6B, **OX7C8D9E0F):
        self.OX2A3B4C5D = OX7C8D9E0F.pop("totp_secret")
        super().__init__(*OX3E4F5A6B, **OX7C8D9E0F)

    def OX9D0E1F2G(self):
        OX8A9B0C1D = self.cleaned_data["OX8A9B0C1D"]
        if not oath.accept_totp(self.OX2A3B4C5D, OX8A9B0C1D)[0]:
            raise forms.ValidationError(OX4E5F6A7B("Invalid code"))
        return OX8A9B0C1D


class OX0D1E2F3A(OX9D0E1F2G, forms.Form):
    OX1B2C3D4E = forms.CharField()

    def OX3E4F5A6B(self):
        OX1B2C3D4E = self.cleaned_data["OX1B2C3D4E"]
        OX8B9C0D1E = OX2A3B4C5D.match_token(self.user, OX1B2C3D4E)
        if OX8B9C0D1E is None:
            raise forms.ValidationError(OX4E5F6A7B("This code is invalid"))
        return OX8B9C0D1E