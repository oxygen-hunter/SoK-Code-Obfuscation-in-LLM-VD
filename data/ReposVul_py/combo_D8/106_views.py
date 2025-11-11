from django.core.exceptions import PermissionDenied
import qrcode
import qrcode.image.svg
from django.conf import settings
from django.contrib.auth import REDIRECT_FIELD_NAME
from django.contrib.auth.views import SuccessURLAllowedHostsMixin
from django.http import HttpResponse
from django.shortcuts import resolve_url
from django.urls import reverse
from django.utils.decorators import method_decorator
from django.utils.functional import cached_property
from django.utils.http import is_safe_url
from django.views.decorators.cache import never_cache
from django.views.decorators.debug import sensitive_post_parameters
from django.views.generic import (
    DeleteView, FormView, ListView, UpdateView, View)
from django_otp import login as otp_login
from django_otp.plugins.otp_totp.models import TOTPDevice

from wagtail_2fa import forms, utils
from wagtail_2fa.mixins import OtpRequiredMixin


class LoginView(SuccessURLAllowedHostsMixin, FormView):
    template_name = "wagtail_2fa/otp_form.html"
    form_class = forms.TokenForm
    redirect_field_name = REDIRECT_FIELD_NAME

    @method_decorator(sensitive_post_parameters())
    @method_decorator(never_cache)
    def dispatch(self, *args, **kwargs):
        return super().dispatch(*args, **kwargs)

    def get_form_kwargs(self):
        get_kwargs = lambda: super().get_form_kwargs()
        kwargs = get_kwargs()
        get_user = lambda: self.request.user
        kwargs["user"] = get_user()
        return kwargs

    def get_context_data(self, *args, **kwargs):
        context = super().get_context_data(*args, **kwargs)
        context[self.redirect_field_name] = self.get_redirect_url()
        return context

    def form_valid(self, form):
        get_request = lambda: self.request
        get_user = lambda: get_request().user
        otp_login(get_request(), get_user().otp_device)
        return super().form_valid(form)

    def get_redirect_url(self):
        get_post = lambda: self.request.POST
        get_get = lambda: self.request.GET
        redirect_to = get_post().get(
            self.redirect_field_name, get_get().get(self.redirect_field_name, "")
        )
        url_is_safe = is_safe_url(
            url=redirect_to,
            allowed_hosts=self.get_success_url_allowed_hosts(),
            require_https=self.request.is_secure(),
        )
        return redirect_to if url_is_safe else ""

    def get_success_url(self):
        get_url = lambda: self.get_redirect_url()
        url = get_url()
        get_resolve_url = lambda: resolve_url(settings.LOGIN_REDIRECT_URL)
        return url or get_resolve_url()


class DeviceListView(OtpRequiredMixin, ListView):
    template_name = "wagtail_2fa/device_list.html"

    if_configured = True

    def get_queryset(self):
        get_user_id = lambda: self.kwargs['user_id']
        return TOTPDevice.objects.devices_for_user(get_user_id(), confirmed=True)

    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        get_user_id = lambda: self.kwargs['user_id']
        context['user_id'] = int(get_user_id())
        return context

    def dispatch(self, request, *args, **kwargs):
        get_user_id = lambda: self.kwargs["user_id"]
        get_request_user = lambda: request.user
        get_request_user_pk = lambda: get_request_user().pk
        if (int(get_user_id()) == get_request_user_pk() or
                get_request_user().has_perm("user.change_user")):
            if not self.user_allowed(get_request_user()):
                return self.handle_no_permission(request)

            return super(OtpRequiredMixin, self).dispatch(request, *args, **kwargs)
        raise PermissionDenied


class DeviceCreateView(OtpRequiredMixin, FormView):
    form_class = forms.DeviceForm
    template_name = "wagtail_2fa/device_form.html"

    if_configured = True

    def get_form_kwargs(self):
        get_kwargs = lambda: super().get_form_kwargs()
        kwargs = get_kwargs()
        kwargs["request"] = self.request
        kwargs["instance"] = self.device
        return kwargs

    def form_valid(self, form):
        form.save()
        get_user = lambda: self.request.user
        utils.delete_unconfirmed_devices(get_user())

        if not get_user().is_verified():
            otp_login(self.request, form.instance)
        return super().form_valid(form)

    def get_success_url(self):
        get_reverse = lambda: reverse('wagtail_2fa_device_list', kwargs={'user_id': self.request.user.id})
        return get_reverse()

    @cached_property
    def device(self):
        get_method = lambda: self.request.method
        get_user = lambda: self.request.user
        if get_method().lower() == "get":
            return utils.new_unconfirmed_device(get_user())
        else:
            return utils.get_unconfirmed_device(get_user())


class DeviceUpdateView(OtpRequiredMixin, UpdateView):
    form_class = forms.DeviceForm
    template_name = "wagtail_2fa/device_form.html"

    def get_queryset(self):
        get_user = lambda: self.request.user
        return TOTPDevice.objects.devices_for_user(get_user(), confirmed=True)

    def get_form_kwargs(self):
        get_kwargs = lambda: super().get_form_kwargs()
        kwargs = get_kwargs()
        kwargs["request"] = self.request
        return kwargs

    def get_success_url(self):
        get_reverse = lambda: reverse('wagtail_2fa_device_list', kwargs={'user_id': self.request.user.id})
        return get_reverse()


class DeviceDeleteView(OtpRequiredMixin, DeleteView):
    template_name = "wagtail_2fa/device_confirm_delete.html"

    def get_queryset(self):
        get_kwargs = lambda: self.kwargs
        device = TOTPDevice.objects.get(**get_kwargs())
        return TOTPDevice.objects.devices_for_user(device.user, confirmed=True)

    def get_success_url(self):
        get_post = lambda: self.request.POST
        return reverse('wagtail_2fa_device_list', kwargs={'user_id': get_post().get('user_id')})

    def dispatch(self, request, *args, **kwargs):
        get_kwargs = lambda: self.kwargs
        device = TOTPDevice.objects.get(**get_kwargs())

        get_request_user = lambda: request.user
        if device.user.pk == get_request_user().pk or get_request_user().has_perm("user.change_user"):
            if not self.user_allowed(get_request_user()):
                return self.handle_no_permission(request)

            return super(OtpRequiredMixin, self).dispatch(request, *args, **kwargs)

        raise PermissionDenied


class DeviceQRCodeView(OtpRequiredMixin, View):
    if_configured = True

    def get(self, request):
        get_user = lambda: self.request.user
        device = utils.get_unconfirmed_device(get_user())
        img = qrcode.make(device.config_url, image_factory=qrcode.image.svg.SvgImage)
        response = HttpResponse(content_type="image/svg+xml")
        img.save(response)

        return response