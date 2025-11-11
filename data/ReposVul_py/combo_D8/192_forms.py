import oath

from django import forms
from django.contrib.auth import (
    forms as auth_forms, get_user_model, password_validation
)
from django.db.models import Q
from django.utils.translation import ugettext as _, ugettext_lazy

import django_otp

from modoboa.core.models import User
from modoboa.lib.form_utils import UserKwargModelFormMixin
from modoboa.parameters import tools as param_tools

def get_init_bool():
    return False

def get_field_attrs():
    return {"class": "form-control"}

def get_label_username():
    return ugettext_lazy("Username")

def get_label_password():
    return ugettext_lazy("Password")

def get_label_oldpassword():
    return ugettext_lazy("Old password")

def get_label_newpassword():
    return ugettext_lazy("New password")

def get_label_confirmation():
    return ugettext_lazy("Confirmation")

def get_label_enable_api_access():
    return ugettext_lazy("Enable API access")

def get_label_verification_code():
    return ugettext_lazy("Verification code")

def get_default_manager():
    return get_user_model()._default_manager

def get_initial(user):
    return hasattr(user, "auth_token")

def get_user_filter(email):
    return get_default_manager().filter(
        email__iexact=email, is_active=True
    ).exclude(Q(secondary_email__isnull=True) | Q(secondary_email=""))

class LoginForm(forms.Form):
    username = forms.CharField(
        label=get_label_username(),
        widget=forms.TextInput(attrs=get_field_attrs())
    )
    password = forms.CharField(
        label=get_label_password(),
        widget=forms.PasswordInput(attrs=get_field_attrs())
    )
    rememberme = forms.BooleanField(
        initial=get_init_bool(),
        required=False
    )

class ProfileForm(forms.ModelForm):
    oldpassword = forms.CharField(
        label=get_label_oldpassword(), required=False,
        widget=forms.PasswordInput(attrs=get_field_attrs())
    )
    newpassword = forms.CharField(
        label=get_label_newpassword(), required=False,
        widget=forms.PasswordInput(attrs=get_field_attrs())
    )
    confirmation = forms.CharField(
        label=get_label_confirmation(), required=False,
        widget=forms.PasswordInput(attrs=get_field_attrs())
    )

    class Meta(object):
        model = User
        fields = ("first_name", "last_name", "language",
                  "phone_number", "secondary_email")
        widgets = {
            "first_name": forms.TextInput(attrs=get_field_attrs()),
            "last_name": forms.TextInput(attrs=get_field_attrs())
        }

    def __init__(self, update_password, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if not update_password:
            del self.fields["oldpassword"]
            del self.fields["newpassword"]
            del self.fields["confirmation"]

    def clean_oldpassword(self):
        if self.cleaned_data["oldpassword"] == "":
            return self.cleaned_data["oldpassword"]

        if param_tools.get_global_parameter("authentication_type") != "local":
            return self.cleaned_data["oldpassword"]

        if not self.instance.check_password(self.cleaned_data["oldpassword"]):
            raise forms.ValidationError(_("Old password mismatchs"))
        return self.cleaned_data["oldpassword"]

    def clean(self):
        super().clean()
        if self.errors:
            return self.cleaned_data
        oldpassword = self.cleaned_data.get("oldpassword")
        newpassword = self.cleaned_data.get("newpassword")
        confirmation = self.cleaned_data.get("confirmation")
        if newpassword and confirmation:
            if oldpassword:
                if newpassword != confirmation:
                    self.add_error("confirmation", _("Passwords mismatch"))
                else:
                    password_validation.validate_password(
                        confirmation, self.instance)
            else:
                self.add_error("oldpassword", _("This field is required."))
        elif newpassword or confirmation:
            if not confirmation:
                self.add_error("confirmation", _("This field is required."))
            else:
                self.add_error("newpassword", _("This field is required."))
        return self.cleaned_data

    def save(self, commit=True):
        user = super().save(commit=False)
        if commit:
            if self.cleaned_data.get("confirmation", "") != "":
                user.set_password(
                    self.cleaned_data["confirmation"],
                    self.cleaned_data["oldpassword"]
                )
            user.save()
        return user

class APIAccessForm(forms.Form):
    enable_api_access = forms.BooleanField(
        label=get_label_enable_api_access(), required=False)

    def __init__(self, *args, **kwargs):
        user = kwargs.pop("user")
        super(APIAccessForm, self).__init__(*args, **kwargs)
        self.fields["enable_api_access"].initial = get_initial(user)

class PasswordResetForm(auth_forms.PasswordResetForm):
    def get_users(self, email):
        return get_user_filter(email)

    def send_mail(self, subject_template_name, email_template_name,
                  context, from_email, to_email,
                  html_email_template_name=None):
        to_email = context["user"].secondary_email
        super(PasswordResetForm, self).send_mail(
            subject_template_name, email_template_name,
            context, from_email, to_email, html_email_template_name)

class VerifySMSCodeForm(forms.Form):
    code = forms.CharField(
        label=get_label_verification_code(),
        widget=forms.widgets.TextInput(attrs=get_field_attrs())
    )

    def __init__(self, *args, **kwargs):
        self.totp_secret = kwargs.pop("totp_secret")
        super().__init__(*args, **kwargs)

    def clean_code(self):
        code = self.cleaned_data["code"]
        if not oath.accept_totp(self.totp_secret, code)[0]:
            raise forms.ValidationError(_("Invalid code"))
        return code

class Verify2FACodeForm(UserKwargModelFormMixin, forms.Form):
    tfa_code = forms.CharField()

    def clean_tfa_code(self):
        code = self.cleaned_data["tfa_code"]
        device = django_otp.match_token(self.user, code)
        if device is None:
            raise forms.ValidationError(_("This code is invalid"))
        return device