# coding: utf-8

from django.conf import settings
from django.contrib import messages
from django.contrib.admin.views.decorators import staff_member_required
from django.contrib.auth import load_backend, login
from django.core.exceptions import ObjectDoesNotExist
from django.http import Http404
from django.shortcuts import redirect
from django.utils.html import escape
from django.utils.http import url_has_allowed_host_and_scheme
from django.utils.translation import gettext_lazy as _

from grappelli.settings import SWITCH_USER_ORIGINAL, SWITCH_USER_TARGET

try:
    from django.contrib.auth import get_user_model
    User = get_user_model()
except ImportError:
    from django.contrib.auth.models import User

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.halted = False

    def run(self):
        while not self.halted and self.pc < len(self.instructions):
            ins, *args = self.instructions[self.pc]
            getattr(self, f"ins_{ins}")(*args)
            self.pc += 1

    def load_program(self, program):
        self.instructions = program

    def ins_PUSH(self, value):
        self.stack.append(value)

    def ins_POP(self):
        return self.stack.pop()

    def ins_JZ(self, offset):
        if self.stack.pop() == 0:
            self.pc += offset - 1

    def ins_JMP(self, offset):
        self.pc += offset - 1

    def ins_CALL(self, func):
        func()

    def ins_HALT(self):
        self.halted = True

@staff_member_required
def switch_user(request, object_id):
    def check_redirect():
        redirect_url = request.GET.get("redirect", None)
        if redirect_url is None or not \
            url_has_allowed_host_and_scheme(
                url=redirect_url,
                allowed_hosts={request.get_host()},
                require_https=request.is_secure(),
            ):
            raise Http404()
        return redirect_url

    def get_user(user_id, error_msg):
        try:
            user = User.objects.get(pk=user_id, is_staff=True)
            return user
        except ObjectDoesNotExist:
            msg = _('%(name)s object with primary key %(key)r does not exist.') % {'name': "User", 'key': escape(user_id)}
            messages.add_message(request, messages.ERROR, msg)
            return None

    def find_backend(target_user):
        if not hasattr(target_user, 'backend'):
            for backend in settings.AUTHENTICATION_BACKENDS:
                if target_user == load_backend(backend).get_user(target_user.pk):
                    target_user.backend = backend
                    break
        return hasattr(target_user, 'backend')

    vm = VM()
    session_user = {"id": request.user.id, "username": request.user.get_username()}
    redirect_url = check_redirect()

    def vm_logic():
        original_user = get_user(session_user["id"], _("Permission denied."))
        if original_user and not SWITCH_USER_ORIGINAL(original_user):
            messages.add_message(request, messages.ERROR, _("Permission denied."))
            vm.ins_PUSH(0)
        else:
            target_user = get_user(object_id, _("Permission denied."))
            if target_user and original_user and target_user != original_user and not SWITCH_USER_TARGET(original_user, target_user):
                messages.add_message(request, messages.ERROR, _("Permission denied."))
                vm.ins_PUSH(0)
            elif target_user and find_backend(target_user):
                login(request, target_user)
                if original_user.id != target_user.id:
                    request.session["original_user"] = {"id": original_user.id, "username": original_user.get_username()}
                vm.ins_PUSH(1)
            else:
                vm.ins_PUSH(0)

    program = [
        ("CALL", vm_logic), 
        ("JZ", 2), 
        ("CALL", lambda: redirect(redirect_url)), 
        ("HALT",)
    ]

    vm.load_program(program)
    vm.run()