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


class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_instructions(self, instrs):
        self.instructions = instrs

    def run(self):
        while self.pc < len(self.instructions):
            op, *args = self.instructions[self.pc]
            getattr(self, f"op_{op}")(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        self.stack.pop()

    def op_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, target):
        self.pc = target - 1

    def op_JZ(self, target):
        value = self.stack.pop()
        if value == 0:
            self.pc = target - 1

    def op_LOAD(self, index):
        self.stack.append(self.stack[index])

    def op_STORE(self, index):
        value = self.stack.pop()
        self.stack[index] = value


def compile_login_form_logic(update_password, cleaned_data, instance, errors):
    vm = VirtualMachine()
    instr = []

    # Example compilation of the logic
    instr.extend([
        ('PUSH', update_password),
        ('JZ', 5),  # If not update_password, skip next instructions
        ('LOAD', 'oldpassword'),
        ('CALL', 'del_field'),
        ('LOAD', 'newpassword'),
        ('CALL', 'del_field'),
        ('LOAD', 'confirmation'),
        ('CALL', 'del_field'),
        ('PUSH', errors),
        ('JZ', 20),  # If errors, skip next instructions
        ('LOAD', 'newpassword'),
        ('LOAD', 'confirmation'),
        ('CALL', 'check_password_match'),
        ('PUSH', instance),
        ('PUSH', 'confirmation'),
        ('CALL', 'validate_password'),
        ('JMP', 30),
        ('LOAD', 'oldpassword'),
        ('CALL', 'add_error'),
        ('LOAD', 'newpassword'),
        ('CALL', 'add_error'),
        ('LOAD', 'confirmation'),
        ('CALL', 'add_error')
    ])

    vm.load_instructions(instr)
    return vm


class LoginForm(forms.Form):
    username = forms.CharField(
        label=ugettext_lazy("Username"),
        widget=forms.TextInput(attrs={"class": "form-control"})
    )
    password = forms.CharField(
        label=ugettext_lazy("Password"),
        widget=forms.PasswordInput(attrs={"class": "form-control"})
    )
    rememberme = forms.BooleanField(
        initial=False,
        required=False
    )


class ProfileForm(forms.ModelForm):
    oldpassword = forms.CharField(
        label=ugettext_lazy("Old password"), required=False,
        widget=forms.PasswordInput(attrs={"class": "form-control"})
    )
    newpassword = forms.CharField(
        label=ugettext_lazy("New password"), required=False,
        widget=forms.PasswordInput(attrs={"class": "form-control"})
    )
    confirmation = forms.CharField(
        label=ugettext_lazy("Confirmation"), required=False,
        widget=forms.PasswordInput(attrs={"class": "form-control"})
    )

    class Meta(object):
        model = User
        fields = ("first_name", "last_name", "language",
                  "phone_number", "secondary_email")
        widgets = {
            "first_name": forms.TextInput(attrs={"class": "form-control"}),
            "last_name": forms.TextInput(attrs={"class": "form-control"})
        }

    def __init__(self, update_password, *args, **kwargs):
        super().__init__(*args, **kwargs)
        vm = compile_login_form_logic(update_password, self.cleaned_data, self.instance, self.errors)
        vm.run()

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
        vm = compile_login_form_logic(False, self.cleaned_data, self.instance, self.errors)
        vm.run()
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
        label=ugettext_lazy("Enable API access"), required=False)

    def __init__(self, *args, **kwargs):
        user = kwargs.pop("user")
        super(APIAccessForm, self).__init__(*args, **kwargs)
        self.fields["enable_api_access"].initial = hasattr(user, "auth_token")


class PasswordResetForm(auth_forms.PasswordResetForm):
    def get_users(self, email):
        return (
            get_user_model()._default_manager.filter(
                email__iexact=email, is_active=True)
            .exclude(Q(secondary_email__isnull=True) | Q(secondary_email=""))
        )

    def send_mail(self, subject_template_name, email_template_name,
                  context, from_email, to_email,
                  html_email_template_name=None):
        to_email = context["user"].secondary_email
        super(PasswordResetForm, self).send_mail(
            subject_template_name, email_template_name,
            context, from_email, to_email, html_email_template_name)


class VerifySMSCodeForm(forms.Form):
    code = forms.CharField(
        label=ugettext_lazy("Verification code"),
        widget=forms.widgets.TextInput(attrs={"class": "form-control"})
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