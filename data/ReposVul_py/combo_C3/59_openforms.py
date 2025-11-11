from django.template import Library
from django.template.defaultfilters import stringfilter

from rest_framework.reverse import reverse

from openforms.config.models import GlobalConfiguration
from openforms.utils.redirect import allow_redirect_url

from ..context_processors import sdk_urls

register = Library()

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []

    def load_program(self, bytecode):
        self.program = bytecode

    def run(self):
        while self.pc < len(self.program):
            op, *args = self.program[self.pc]
            getattr(self, f'op_{op}')(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a - b)

    def op_JMP(self, addr):
        self.pc = addr - 1

    def op_JZ(self, addr):
        if not self.op_POP():
            self.pc = addr - 1

    def op_LOAD(self, index):
        self.op_PUSH(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.op_POP()

    def op_CALL(self, func, *args):
        self.op_PUSH(func(*args))

def api_base_url(context: dict):
    vm = VM()
    vm.load_program([
        ('PUSH', context),
        ('LOAD', 0),
        ('CALL', reverse, "api:api-root"),
        ('LOAD', 1),
        ('CALL', lambda request, api_root: request.build_absolute_uri(api_root), vm.op_POP(), vm.op_POP()),
        ('POP',)
    ])
    vm.run()
    return vm.op_POP()

@register.simple_tag(takes_context=True)
def api_base_url_tag(context: dict):
    return api_base_url(context)

def trim(value):
    vm = VM()
    vm.load_program([
        ('PUSH', value),
        ('CALL', str.strip, vm.op_POP()),
        ('POP',)
    ])
    vm.run()
    return vm.op_POP()

@register.filter
@stringfilter
def trim_tag(value):
    return trim(value)

def sdk_info_banner():
    vm = VM()
    vm.load_program([
        ('CALL', GlobalConfiguration.get_solo),
        ('STORE', 0),
        ('LOAD', 0),
        ('CALL', lambda config: config.display_sdk_information, vm.op_POP()),
        ('STORE', 1),
        ('CALL', sdk_urls, None),
        ('STORE', 2),
        ('LOAD', 1),
        ('LOAD', 2),
        ('CALL', lambda enabled, urls: {"enabled": enabled, **urls}, vm.op_POP(), vm.op_POP()),
        ('POP',)
    ])
    vm.run()
    return vm.op_POP()

@register.inclusion_tag("forms/sdk_info_banner.html")
def sdk_info_banner_tag():
    return sdk_info_banner()

def get_allowed_redirect_url(*candidates: str) -> str:
    vm = VM()
    vm.load_program([
        ('PUSH', candidates),
        ('CALL', list, vm.op_POP()),
        ('STORE', 0),
        ('PUSH', 0),
        ('STORE', 1),
        ('JMP', 8),
        ('LOAD', 0),
        ('LOAD', 1),
        ('CALL', lambda candidates, i: candidates[i] if i < len(candidates) else "", vm.op_POP(), vm.op_POP()),
        ('STORE', 2),
        ('LOAD', 2),
        ('JZ', 13),
        ('LOAD', 2),
        ('CALL', allow_redirect_url, vm.op_POP()),
        ('JZ', 12),
        ('LOAD', 2),
        ('POP',),
        ('JMP', 15),
        ('PUSH', ''),
        ('POP',)
    ])
    vm.run()
    return vm.op_POP()

@register.simple_tag
def get_allowed_redirect_url_tag(*candidates: str) -> str:
    return get_allowed_redirect_url(*candidates)