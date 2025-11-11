import qrcode
import qrcode.image.svg
from django.core.exceptions import PermissionDenied
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
from django.views.generic import DeleteView, FormView, ListView, UpdateView, View
from django_otp import login as otp_login
from django_otp.plugins.otp_totp.models import TOTPDevice
from wagtail_2fa import forms, utils
from wagtail_2fa.mixins import OtpRequiredMixin

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self, instructions):
        self.instructions = instructions
        self.pc = 0
        while self.pc < len(self.instructions):
            op, *args = self.instructions[self.pc]
            getattr(self, f'op_{op}')(*args)

    def op_PUSH(self, value):
        self.stack.append(value)
        self.pc += 1

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        a = self.op_POP()
        b = self.op_POP()
        self.op_PUSH(a + b)
        self.pc += 1

    def op_SUB(self):
        a = self.op_POP()
        b = self.op_POP()
        self.op_PUSH(a - b)
        self.pc += 1

    def op_JMP(self, address):
        self.pc = address

    def op_JZ(self, address):
        if self.op_POP() == 0:
            self.pc = address
        else:
            self.pc += 1

    def op_LOAD(self, index):
        self.op_PUSH(self.stack[index])
        self.pc += 1

    def op_STORE(self, index):
        self.stack[index] = self.op_POP()
        self.pc += 1

    def op_CALL(self, func_name, *args):
        func = globals().get(func_name)
        if func:
            func(self, *args)
        self.pc += 1

class LoginView(SuccessURLAllowedHostsMixin, FormView):
    template_name = "wagtail_2fa/otp_form.html"
    form_class = forms.TokenForm
    redirect_field_name = REDIRECT_FIELD_NAME

    @method_decorator(sensitive_post_parameters())
    @method_decorator(never_cache)
    def dispatch(self, *args, **kwargs):
        vm = VM()
        vm.run([
            ('CALL', '_dispatch', self, args, kwargs)
        ])
        return vm.op_POP()

    def _dispatch(vm, self, args, kwargs):
        vm.op_PUSH(super(LoginView, self).dispatch(*args, **kwargs))

    def get_form_kwargs(self):
        vm = VM()
        vm.run([
            ('CALL', '_get_form_kwargs', self)
        ])
        return vm.op_POP()

    def _get_form_kwargs(vm, self):
        kwargs = super(LoginView, self).get_form_kwargs()
        kwargs["user"] = self.request.user
        vm.op_PUSH(kwargs)

    def get_context_data(self, *args, **kwargs):
        vm = VM()
        vm.run([
            ('CALL', '_get_context_data', self, args, kwargs)
        ])
        return vm.op_POP()

    def _get_context_data(vm, self, args, kwargs):
        context = super(LoginView, self).get_context_data(*args, **kwargs)
        context[self.redirect_field_name] = self.get_redirect_url()
        vm.op_PUSH(context)

    def form_valid(self, form):
        vm = VM()
        vm.run([
            ('CALL', '_form_valid', self, form)
        ])
        return vm.op_POP()

    def _form_valid(vm, self, form):
        otp_login(self.request, self.request.user.otp_device)
        vm.op_PUSH(super(LoginView, self).form_valid(form))

    def get_redirect_url(self):
        vm = VM()
        vm.run([
            ('CALL', '_get_redirect_url', self)
        ])
        return vm.op_POP()

    def _get_redirect_url(vm, self):
        redirect_to = self.request.POST.get(
            self.redirect_field_name, self.request.GET.get(self.redirect_field_name, "")
        )
        url_is_safe = is_safe_url(
            url=redirect_to,
            allowed_hosts=self.get_success_url_allowed_hosts(),
            require_https=self.request.is_secure(),
        )
        vm.op_PUSH(redirect_to if url_is_safe else "")

    def get_success_url(self):
        vm = VM()
        vm.run([
            ('CALL', '_get_success_url', self)
        ])
        return vm.op_POP()

    def _get_success_url(vm, self):
        url = self.get_redirect_url()
        vm.op_PUSH(url or resolve_url(settings.LOGIN_REDIRECT_URL))

class DeviceListView(OtpRequiredMixin, ListView):
    template_name = "wagtail_2fa/device_list.html"
    if_configured = True

    def get_queryset(self):
        vm = VM()
        vm.run([
            ('CALL', '_get_queryset', self)
        ])
        return vm.op_POP()

    def _get_queryset(vm, self):
        vm.op_PUSH(TOTPDevice.objects.devices_for_user(self.kwargs['user_id'], confirmed=True))

    def get_context_data(self, **kwargs):
        vm = VM()
        vm.run([
            ('CALL', '_get_context_data', self, kwargs)
        ])
        return vm.op_POP()

    def _get_context_data(vm, self, kwargs):
        context = super(DeviceListView, self).get_context_data(**kwargs)
        context['user_id'] = int(self.kwargs['user_id'])
        vm.op_PUSH(context)

    def dispatch(self, request, *args, **kwargs):
        vm = VM()
        vm.run([
            ('CALL', '_dispatch', self, request, args, kwargs)
        ])
        return vm.op_POP()

    def _dispatch(vm, self, request, args, kwargs):
        if (int(self.kwargs["user_id"]) == request.user.pk or
                request.user.has_perm("user.change_user")):
            if not self.user_allowed(request.user):
                vm.op_PUSH(self.handle_no_permission(request))
                return
            vm.op_PUSH(super(OtpRequiredMixin, self).dispatch(request, *args, **kwargs))
            return
        raise PermissionDenied

class DeviceCreateView(OtpRequiredMixin, FormView):
    form_class = forms.DeviceForm
    template_name = "wagtail_2fa/device_form.html"
    if_configured = True

    def get_form_kwargs(self):
        vm = VM()
        vm.run([
            ('CALL', '_get_form_kwargs', self)
        ])
        return vm.op_POP()

    def _get_form_kwargs(vm, self):
        kwargs = super(DeviceCreateView, self).get_form_kwargs()
        kwargs["request"] = self.request
        kwargs["instance"] = self.device
        vm.op_PUSH(kwargs)

    def form_valid(self, form):
        vm = VM()
        vm.run([
            ('CALL', '_form_valid', self, form)
        ])
        return vm.op_POP()

    def _form_valid(vm, self, form):
        form.save()
        utils.delete_unconfirmed_devices(self.request.user)
        if not self.request.user.is_verified():
            otp_login(self.request, form.instance)
        vm.op_PUSH(super(DeviceCreateView, self).form_valid(form))

    def get_success_url(self):
        vm = VM()
        vm.run([
            ('CALL', '_get_success_url', self)
        ])
        return vm.op_POP()

    def _get_success_url(vm, self):
        vm.op_PUSH(reverse('wagtail_2fa_device_list', kwargs={'user_id': self.request.user.id}))

    @cached_property
    def device(self):
        vm = VM()
        vm.run([
            ('CALL', '_device', self)
        ])
        return vm.op_POP()

    def _device(vm, self):
        if self.request.method.lower() == "get":
            vm.op_PUSH(utils.new_unconfirmed_device(self.request.user))
        else:
            vm.op_PUSH(utils.get_unconfirmed_device(self.request.user))

class DeviceUpdateView(OtpRequiredMixin, UpdateView):
    form_class = forms.DeviceForm
    template_name = "wagtail_2fa/device_form.html"

    def get_queryset(self):
        vm = VM()
        vm.run([
            ('CALL', '_get_queryset', self)
        ])
        return vm.op_POP()

    def _get_queryset(vm, self):
        vm.op_PUSH(TOTPDevice.objects.devices_for_user(self.request.user, confirmed=True))

    def get_form_kwargs(self):
        vm = VM()
        vm.run([
            ('CALL', '_get_form_kwargs', self)
        ])
        return vm.op_POP()

    def _get_form_kwargs(vm, self):
        kwargs = super(DeviceUpdateView, self).get_form_kwargs()
        kwargs["request"] = self.request
        vm.op_PUSH(kwargs)

    def get_success_url(self):
        vm = VM()
        vm.run([
            ('CALL', '_get_success_url', self)
        ])
        return vm.op_POP()

    def _get_success_url(vm, self):
        vm.op_PUSH(reverse('wagtail_2fa_device_list', kwargs={'user_id': self.request.user.id}))

class DeviceDeleteView(OtpRequiredMixin, DeleteView):
    template_name = "wagtail_2fa/device_confirm_delete.html"

    def get_queryset(self):
        vm = VM()
        vm.run([
            ('CALL', '_get_queryset', self)
        ])
        return vm.op_POP()

    def _get_queryset(vm, self):
        device = TOTPDevice.objects.get(**self.kwargs)
        vm.op_PUSH(TOTPDevice.objects.devices_for_user(device.user, confirmed=True))

    def get_success_url(self):
        vm = VM()
        vm.run([
            ('CALL', '_get_success_url', self)
        ])
        return vm.op_POP()

    def _get_success_url(vm, self):
        vm.op_PUSH(reverse('wagtail_2fa_device_list', kwargs={'user_id': self.request.POST.get('user_id')}))

    def dispatch(self, request, *args, **kwargs):
        vm = VM()
        vm.run([
            ('CALL', '_dispatch', self, request, args, kwargs)
        ])
        return vm.op_POP()

    def _dispatch(vm, self, request, args, kwargs):
        device = TOTPDevice.objects.get(**self.kwargs)
        if device.user.pk == request.user.pk or request.user.has_perm("user.change_user"):
            if not self.user_allowed(request.user):
                vm.op_PUSH(self.handle_no_permission(request))
                return
            vm.op_PUSH(super(OtpRequiredMixin, self).dispatch(request, *args, **kwargs))
            return
        raise PermissionDenied

class DeviceQRCodeView(OtpRequiredMixin, View):
    if_configured = True

    def get(self, request):
        vm = VM()
        vm.run([
            ('CALL', '_get', self, request)
        ])
        return vm.op_POP()

    def _get(vm, self, request):
        device = utils.get_unconfirmed_device(self.request.user)
        img = qrcode.make(device.config_url, image_factory=qrcode.image.svg.SvgImage)
        response = HttpResponse(content_type="image/svg+xml")
        img.save(response)
        vm.op_PUSH(response)