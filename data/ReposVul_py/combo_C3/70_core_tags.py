"""Custom tags for Core application."""
import os
import re
from functools import reduce

import pkg_resources

from django import template
from django.conf import settings
from django.contrib.sessions.models import Session
from django.template.loader import render_to_string
from django.urls import reverse
from django.utils import timezone
from django.utils.encoding import smart_str
from django.utils.safestring import mark_safe
from django.utils.translation import get_language, gettext as _

from .. import models
from .. import signals

register = template.Library()


class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            self.pc += 1
            getattr(self, f'op_{opcode}')(*args)

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

    def op_LOAD(self, index):
        self.stack.append(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.stack.pop()

    def op_JMP(self, target):
        self.pc = target

    def op_JZ(self, target):
        if self.stack.pop() == 0:
            self.pc = target

    def op_CALL(self, func_name):
        self.stack.append(globals()[func_name](*self.stack.pop()))

    def op_RET(self):
        return self.stack.pop()


def core_menu_vm(selection, user):
    vm = VM()
    vm.run([
        ('PUSH', signals.extra_admin_menu_entries.send(sender="core_menu", location="top_menu", user=user)),
        ('CALL', 'reduce'),
        ('STORE', 0),
        ('LOAD', 0),
        ('CALL', 'len'),
        ('PUSH', 0),
        ('SUB',),
        ('JZ', 6),
        ('LOAD', 0),
        ('PUSH', [{"name": "settings", "label": _("Modoboa"), "url": reverse("core:index")}]),
        ('ADD',),
        ('STORE', 0),
        ('LOAD', 0),
        ('CALL', 'render_to_string', "common/menulist.html", {"entries": vm.stack.pop(), "selection": selection, "user": user}),
        ('RET',)
    ])
    return vm.stack.pop()


def extensions_menu_vm(selection, user):
    vm = VM()
    vm.run([
        ('PUSH', signals.extra_user_menu_entries.send(sender="core_menu", location="top_menu", user=user)),
        ('CALL', 'reduce'),
        ('STORE', 0),
        ('LOAD', 0),
        ('CALL', 'render_to_string', "common/menulist.html", {"selection": selection, "entries": vm.stack.pop(), "user": user}),
        ('RET',)
    ])
    return vm.stack.pop()


def admin_menu_vm(selection, user):
    vm = VM()
    vm.run([
        ('PUSH', [
            {"name": "info", "class": "ajaxnav", "url": "info/", "label": _("Information")},
            {"name": "logs", "class": "ajaxnav", "url": "logs/?sort_order=-date_created", "label": _("Logs")},
            {"name": "parameters", "class": "ajaxnav", "url": "parameters/", "img": "", "label": _("Parameters")},
        ]),
        ('CALL', 'render_to_string', "common/menu.html", {"entries": vm.stack.pop(), "css": "nav nav-sidebar", "selection": selection, "user": user}),
        ('RET',)
    ])
    return vm.stack.pop()


def user_menu_vm(request, selection):
    vm = VM()
    vm.run([
        ('PUSH', [
            {"name": "user", "img": "fa fa-user", "label": request.user.fullname, "menu": [
                {"name": "settings", "img": "fa fa-list", "label": _("Settings"), "url": reverse("core:user_index")}
            ]}
        ]),
        ('PUSH', signals.extra_user_menu_entries.send(sender="user_menu", location="options_menu", user=request.user)),
        ('CALL', 'reduce'),
        ('ADD',),
        ('PUSH', [{
            "name": "logout", "url": reverse("core:logout"), "label": _("Logout"), "img": "fa fa-sign-out", "method": "post"
        }]),
        ('ADD',),
        ('CALL', 'render_to_string', "common/menulist.html", {"request": request, "selection": selection, "entries": vm.stack.pop(), "user": request.user}),
        ('RET',)
    ])
    return vm.stack.pop()


def uprefs_menu_vm(selection, user):
    vm = VM()
    vm.run([
        ('PUSH', [
            {"name": "profile", "class": "ajaxnav", "url": "profile/", "label": _("Profile")},
            {"name": "preferences", "class": "ajaxnav", "url": "preferences/", "label": _("Preferences")},
            {"name": "security", "class": "ajaxnav", "url": "security/", "label": _("Security")},
        ]),
        ('PUSH', 0),
        ('JZ', 8),
        ('PUSH', {
            "name": "api", "class": "ajaxnav", "url": "api/", "label": _("API"),
        }),
        ('ADD',),
        ('LOAD', 0),
        ('PUSH', signals.extra_user_menu_entries.send(sender="user_menu", location="uprefs_menu", user=user)),
        ('CALL', 'reduce'),
        ('ADD',),
        ('STORE', 0),
        ('LOAD', 0),
        ('CALL', 'sorted', key=lambda e: e["label"]),
        ('CALL', 'render_to_string', "common/menu.html", {"entries": vm.stack.pop(), "css": "nav nav-sidebar", "selection": selection, "user": user}),
        ('RET',)
    ])
    return vm.stack.pop()


def colorize_level_vm(level):
    vm = VM()
    vm.run([
        ('PUSH', {
            "INFO": "text-info",
            "WARNING": "text-warning",
            "CRITICAL": "text-danger"
        }),
        ('LOAD', 0),
        ('CALL', 'in', level),
        ('JZ', 5),
        ('PUSH', level),
        ('RET',),
        ('LOAD', 0),
        ('PUSH', level),
        ('ADD',),
        ('CALL', "format", "<p class='%s'>%s</p>"),
        ('RET',)
    ])
    return vm.stack.pop()


def tohtml_vm(message):
    vm = VM()
    vm.run([
        ('CALL', 're.sub', r"'(.*?)'", r"<strong>\g<1></strong>", message),
        ('RET',)
    ])
    return vm.stack.pop()


def visirule_vm(field):
    vm = VM()
    vm.run([
        ('PUSH', hasattr(field, "form")),
        ('PUSH', hasattr(field.form, "visirules")),
        ('PUSH', field.html_name in field.form.visirules),
        ('ADD',),
        ('ADD',),
        ('JZ', 5),
        ('PUSH', ""),
        ('RET',),
        ('PUSH', field.form.visirules[field.html_name]),
        ('CALL', 'format', " data-visibility-field='{}' data-visibility-value='{}' ", vm.stack.pop()["field"], vm.stack.pop()["value"]),
        ('CALL', 'mark_safe', vm.stack.pop()),
        ('RET',)
    ])
    return vm.stack.pop()


def get_version_vm():
    vm = VM()
    vm.run([
        ('CALL', 'pkg_resources.get_distribution', "modoboa"),
        ('LOAD', 0),
        ('RET',)
    ])
    return vm.stack.pop().version


def connected_users_vm(parser, token):
    vm = VM()
    vm.run([
        ('PUSH', token.split_contents()),
        ('CALL', 'len'),
        ('PUSH', 3),
        ('SUB',),
        ('JZ', 6),
        ('CALL', 'template.TemplateSyntaxError', "connected_users usage: {% connected_users as users %}"),
        ('PUSH', None),
        ('RET',),
        ('PUSH', ConnectedUsers(vm.stack.pop())),
        ('RET',)
    ])
    return vm.stack.pop()


def get_modoboa_logo_vm():
    vm = VM()
    vm.run([
        ('PUSH', settings.MODOBOA_CUSTOM_LOGO),
        ('CALL', 'AttributeError'),
        ('JZ', 3),
        ('PUSH', os.path.join(settings.STATIC_URL, "css/modoboa.png")),
        ('RET',),
        ('LOAD', 0),
        ('RET',)
    ])
    return vm.stack.pop()


def load_optionalmenu_vm(user):
    vm = VM()
    vm.run([
        ('PUSH', signals.extra_user_menu_entries.send(sender="user_menu", location="top_menu_middle", user=user)),
        ('CALL', 'reduce'),
        ('CALL', 'template.loader.render_to_string', "common/menulist.html", {"entries": vm.stack.pop(), "user": user}),
        ('RET',)
    ])
    return vm.stack.pop()


def display_messages_vm(msgs):
    vm = VM()
    vm.run([
        ('PUSH', ""),
        ('STORE', 0),
        ('PUSH', "info"),
        ('STORE', 1),
        ('PUSH', msgs),
        ('CALL', 'len'),
        ('JZ', 10),
        ('LOAD', 0),
        ('PUSH', smart_str(msgs.pop(0)) + "\\\n"),
        ('ADD',),
        ('STORE', 0),
        ('LOAD', 1),
        ('PUSH', msgs.pop(0).tags),
        ('STORE', 1),
        ('LOAD', 0),
        ('CALL', 'in', "info"),
        ('JZ', 16),
        ('PUSH', "success"),
        ('STORE', 1),
        ('PUSH', "2000"),
        ('RET',),
        ('PUSH', "undefined"),
        ('RET',),
        ('CALL', 'format', """
<script type="text/javascript">
    $(document).ready(function() {
        $('body').notify('%s', '%s', %s);
    });
</script>
        """, vm.stack.pop(), vm.stack.pop(), vm.stack.pop()),
        ('RET',)
    ])
    return mark_safe(vm.stack.pop())


def currencyfmt_vm(amount):
    vm = VM()
    vm.run([
        ('CALL', 'get_language'),
        ('LOAD', 0),
        ('CALL', 'in', "fr"),
        ('JZ', 5),
        ('PUSH', u"{} €".format(amount)),
        ('RET',),
        ('PUSH', u"€{}".format(amount)),
        ('RET',)
    ])
    return vm.stack.pop()

core_menu = core_menu_vm
extensions_menu = extensions_menu_vm
admin_menu = admin_menu_vm
user_menu = user_menu_vm
uprefs_menu = uprefs_menu_vm
colorize_level = colorize_level_vm
tohtml = tohtml_vm
visirule = visirule_vm
get_version = get_version_vm
connected_users = connected_users_vm
get_modoboa_logo = get_modoboa_logo_vm
load_optionalmenu = load_optionalmenu_vm
display_messages = display_messages_vm
currencyfmt = currencyfmt_vm