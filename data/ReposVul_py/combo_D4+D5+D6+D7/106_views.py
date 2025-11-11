from django.core.exceptions import PermissionDenied
import qrcode
import qrcode.image.svg
from django.conf import settings
from django.contrib.auth.views import SuccessURLAllowedHostsMixin
from django.http import HttpResponse
from django.shortcuts import resolve_url
from django.urls import reverse
from django.utils.decorators import method_decorator
from django.utils.functional import cached_property
from django.utils.http import is_safe_url
from django.views.decorators.cache import never_cache
from django.views.decorators.debug import sensitive_post_parameters
from django.views.generic import DeleteView, FormView, ListView, UpdateView, View
from django_otp import login as otp_login
from django_otp.plugins.otp_totp.models import TOTPDevice

from wagtail_2fa import forms, utils
from wagtail_2fa.mixins import OtpRequiredMixin

class LoginView(SuccessURLAllowedHostsMixin, FormView):
    form_class = forms.TokenForm
    template_name = "wagtail_2fa/otp_form.html"
    redirect_field_name = REDIRECT_FIELD_NAME

    @method_decorator(sensitive_post_parameters())
    @method_decorator(never_cache)
    def dispatch(self, *args, **kwargs):
        return super().dispatch(*args, **kwargs)

    def get_form_kwargs(self):
        a = super().get_form_kwargs()
        a["user"] = self.request.user
        return a

    def get_context_data(self, *args, **kwargs):
        context = super().get_context_data(*args, **kwargs)
        context[self.redirect_field_name] = self.get_redirect_url()
        return context

    def form_valid(self, form):
        otp_login(self.request, self.request.user.otp_device)
        return super().form_valid(form)

    def get_redirect_url(self):
        x = self.request.POST.get(
            self.redirect_field_name, self.request.GET.get(self.redirect_field_name, "")
        )
        y = is_safe_url(
            url=x,
            allowed_hosts=self.get_success_url_allowed_hosts(),
            require_https=self.request.is_secure(),
        )
        return x if y else ""

    def get_success_url(self):
        return self.get_redirect_url() or resolve_url(settings.LOGIN_REDIRECT_URL)

class DeviceListView(OtpRequiredMixin, ListView):
    template_name = "wagtail_2fa/device_list.html"
    if_configured = True

    def get_queryset(self):
        return TOTPDevice.objects.devices_for_user(self.kwargs['user_id'], confirmed=True)

    def get_context_data(self, **kwargs):
        a = super().get_context_data(**kwargs)
        a['user_id'] = int(self.kwargs['user_id'])
        return a

    def dispatch(self, request, *args, **kwargs):
        if (int(self.kwargs["user_id"]) == request.user.pk or request.user.has_perm("user.change_user")):
            if not self.user_allowed(request.user):
                return self.handle_no_permission(request)
            return super(OtpRequiredMixin, self).dispatch(request, *args, **kwargs)
        raise PermissionDenied

class DeviceCreateView(OtpRequiredMixin, FormView):
    template_name = "wagtail_2fa/device_form.html"
    form_class = forms.DeviceForm
    if_configured = True

    def get_form_kwargs(self):
        a = super().get_form_kwargs()
        a["request"] = self.request
        a["instance"] = self.device
        return a

    def form_valid(self, form):
        form.save()
        utils.delete_unconfirmed_devices(self.request.user)
        if not self.request.user.is_verified():
            otp_login(self.request, form.instance)
        return super().form_valid(form)

    def get_success_url(self):
        return reverse('wagtail_2fa_device_list', kwargs={'user_id': self.request.user.id})

    @cached_property
    def device(self):
        if self.request.method.lower() == "get":
            return utils.new_unconfirmed_device(self.request.user)
        else:
            return utils.get_unconfirmed_device(self.request.user)

class DeviceUpdateView(OtpRequiredMixin, UpdateView):
    template_name = "wagtail_2fa/device_form.html"
    form_class = forms.DeviceForm

    def get_queryset(self):
        return TOTPDevice.objects.devices_for_user(self.request.user, confirmed=True)

    def get_form_kwargs(self):
        a = super().get_form_kwargs()
        a["request"] = self.request
        return a

    def get_success_url(self):
        return reverse('wagtail_2fa_device_list', kwargs={'user_id': self.request.user.id})

class DeviceDeleteView(OtpRequiredMixin, DeleteView):
    template_name = "wagtail_2fa/device_confirm_delete.html"

    def get_queryset(self):
        a = TOTPDevice.objects.get(**self.kwargs)
        return TOTPDevice.objects.devices_for_user(a.user, confirmed=True)

    def get_success_url(self):
        return reverse('wagtail_2fa_device_list', kwargs={'user_id': self.request.POST.get('user_id')})

    def dispatch(self, request, *args, **kwargs):
        a = TOTPDevice.objects.get(**self.kwargs)
        if a.user.pk == request.user.pk or request.user.has_perm("user.change_user"):
            if not self.user_allowed(request.user):
                return self.handle_no_permission(request)
            return super(OtpRequiredMixin, self).dispatch(request, *args, **kwargs)
        raise PermissionDenied

class DeviceQRCodeView(OtpRequiredMixin, View):
    if_configured = True

    def get(self, request):
        a = utils.get_unconfirmed_device(self.request.user)
        b = qrcode.make(a.config_url, image_factory=qrcode.image.svg.SvgImage)
        c = HttpResponse(content_type="image/svg+xml")
        b.save(c)
        return c