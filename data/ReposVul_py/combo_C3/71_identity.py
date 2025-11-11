from reversion import revisions as reversion

from django.contrib.auth import mixins as auth_mixins
from django.shortcuts import render
from django.template.loader import render_to_string
from django.utils.translation import ugettext as _, ungettext
from django.views import generic
from django.views.decorators.csrf import ensure_csrf_cookie
from django.views.decorators.http import require_http_methods
from modoboa.core.models import User
from modoboa.lib.exceptions import BadRequest, PermDeniedException
from modoboa.lib.listing import get_listing_page, get_sort_order
from modoboa.lib.web_utils import render_to_json_response
from .. import signals
from ..forms import AccountForm, AccountWizard
from ..lib import get_identities
from ..models import Domain, Mailbox

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.context = {}

    def run(self):
        while self.pc < len(self.instructions):
            self.execute(self.instructions[self.pc])
            self.pc += 1

    def execute(self, instr):
        if instr == "PUSH":
            self.stack.append(self.next_value())
        elif instr == "POP":
            self.stack.pop()
        elif instr == "ADD":
            b, a = self.stack.pop(), self.stack.pop()
            self.stack.append(a + b)
        elif instr == "SUB":
            b, a = self.stack.pop(), self.stack.pop()
            self.stack.append(a - b)
        elif instr == "JMP":
            self.pc = self.next_value() - 1
        elif instr == "JZ":
            target = self.next_value()
            if self.stack.pop() == 0:
                self.pc = target - 1
        elif instr == "LOAD":
            self.stack.append(self.context[self.next_value()])
        elif instr == "STORE":
            self.context[self.next_value()] = self.stack.pop()
        else:
            raise ValueError("Unknown instruction")

    def next_value(self):
        self.pc += 1
        return self.instructions[self.pc]

def identities_vm(request):
    vm = VM()
    vm.context.update({
        "searchquery": request.GET.get("searchquery", None),
        "idtfilter": request.GET.get("idtfilter", None),
        "grpfilter": request.GET.get("grpfilter", None),
        "user": request.user,
        "request": request
    })
    vm.instructions = [
        "PUSH", "searchquery", "LOAD",
        "PUSH", "idtfilter", "LOAD",
        "PUSH", "grpfilter", "LOAD",
        "STORE", "filters",
        "PUSH", "filters", "LOAD",
        "PUSH", "user", "LOAD",
        "CALL", "get_identities",
        "STORE", "idents_list",
        "PUSH", "identity", "PUSH", ["identity", "name_or_rcpt", "tags"],
        "CALL", "get_sort_order",
        "STORE", "sort_order", "STORE", "sort_dir",
        "PUSH", "sort_order", "LOAD",
        "PUSH", "identity",
        "EQ", "JZ", 60,
        "PUSH", "idents_list", "LOAD",
        "PUSH", "sort_order", "LOAD",
        "PUSH", "sort_dir", "LOAD",
        "PUSH", "-",
        "CALL", "sorted",
        "STORE", "objects",
        "JMP", 80,
        "LABEL", 60,
        "PUSH", "idents_list", "LOAD",
        "PUSH", "tags", "PUSH", 0, "PUSH", "label",
        "CALL", "sorted",
        "STORE", "objects",
        "LABEL", 80,
        "PUSH", "objects", "LOAD",
        "PUSH", 1,
        "CALL", "get_listing_page",
        "STORE", "page",
        "PUSH", "page", "LOAD",
        "PUSH", None,
        "EQ", "JZ", 140,
        "PUSH", 0,
        "STORE", "context.length",
        "JMP", 180,
        "LABEL", 140,
        "PUSH", "page", "LOAD",
        "PUSH", "object_list",
        "CALL", "render_to_string",
        "STORE", "context.rows",
        "PUSH", "page", "LOAD",
        "PUSH", "number",
        "STORE", "context.pages",
        "LABEL", 180,
        "CALL", "render_to_json_response",
        "RETURN"
    ]
    return vm.run()

def quotas_vm(request):
    vm = VM()
    vm.context.update({
        "searchquery": request.GET.get("searchquery", None),
        "user": request.user,
        "request": request
    })
    vm.instructions = [
        "CALL", "get_sort_order", "STORE", "sort_order", "STORE", "sort_dir",
        "LOAD", "user", "LOAD", "searchquery",
        "CALL", "Mailbox.objects.get_for_admin", "STORE", "mboxes",
        "LOAD", "mboxes", "CALL", "exclude", "STORE", "mboxes",
        "LOAD", "sort_order", "PUSH", "address",
        "EQ", "JZ", 40,
        "LOAD", "sort_order", "PUSH", "quota",
        "EQ", "JZ", 40,
        "LOAD", "mboxes", "LOAD", "sort_dir", "LOAD", "sort_order",
        "CALL", "order_by", "STORE", "mboxes",
        "JMP", 100,
        "LABEL", 40,
        "LOAD", "sort_order", "PUSH", "quota_value__bytes",
        "EQ", "JZ", 60,
        "LOAD", "sort_order", "PUSH", "quota_usage",
        "EQ", "JZ", 60,
        "RAISE", "BadRequest", "Invalid request",
        "LABEL", 60,
        "CALL", "db_type", "STORE", "db_type",
        "LOAD", "db_type", "PUSH", "mysql",
        "EQ", "JZ", 80,
        "PUSH", "CONCAT(admin_mailbox.address,'@',admin_domain.name)",
        "STORE", "where",
        "JMP", 90,
        "LABEL", 80,
        "PUSH", "admin_mailbox.address||'@'||admin_domain.name",
        "STORE", "where",
        "LABEL", 90,
        "LOAD", "sort_order", "PUSH", "quota_value__bytes",
        "EQ", "JZ", 120,
        "LOAD", "mboxes", "LOAD", "sort_dir", "LOAD", "sort_order",
        "CALL", "extra", "STORE", "mboxes",
        "JMP", 150,
        "LABEL", 120,
        "LOAD", "db_type", "PUSH", "postgres",
        "EQ", "JZ", 130,
        "PUSH", "(admin_quota.bytes::float / (CAST(admin_mailbox.quota AS BIGINT) * 1048576)) * 100",
        "STORE", "select",
        "JMP", 140,
        "LABEL", 130,
        "PUSH", "(admin_quota.bytes * 1.0 / (admin_mailbox.quota * 1048576)) * 100",
        "STORE", "select",
        "LABEL", 140,
        "LOAD", "mboxes", "LOAD", "sort_dir", "LOAD", "sort_order",
        "CALL", "extra", "STORE", "mboxes",
        "LABEL", 150,
        "PUSH", "mboxes", "LOAD",
        "PUSH", 1,
        "CALL", "get_listing_page",
        "STORE", "page",
        "PUSH", "page", "LOAD",
        "PUSH", None,
        "EQ", "JZ", 200,
        "PUSH", 0,
        "STORE", "context.length",
        "JMP", 240,
        "LABEL", 200,
        "PUSH", "page", "LOAD",
        "PUSH", "mboxes",
        "CALL", "render_to_string",
        "STORE", "context.rows",
        "PUSH", "page", "LOAD",
        "PUSH", "number",
        "STORE", "context.pages",
        "LABEL", 240,
        "CALL", "render_to_json_response",
        "RETURN"
    ]
    return vm.run()

@login_required
@user_passes_test(
    lambda u: u.has_perm("core.add_user") or
    u.has_perm("admin.add_alias") or
    u.has_perm("admin.add_mailbox")
)
def get_next_page(request):
    if request.GET.get("objtype", "identity") == "identity":
        return identities_vm(request)
    return quotas_vm(request)