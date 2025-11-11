import warnings
import six
from django.http import HttpResponse
from django.utils.crypto import constant_time_compare
from django.utils.decorators import method_decorator
from django.views.decorators.csrf import csrf_exempt
from django.views.generic import View
from ..exceptions import AnymailInsecureWebhookWarning, AnymailWebhookValidationFailure
from ..utils import get_anymail_setting, collect_all_methods, get_request_basic_auth

# VM instruction constants
PUSH = 0
POP = 1
ADD = 2
SUB = 3
JMP = 4
JZ = 5
LOAD = 6
STORE = 7
EXEC_FUNC = 8

# Simple stack-based virtual machine
class SimpleVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            opcode = self.instructions[self.pc]
            self.pc += 1
            if opcode == PUSH:
                value = self.instructions[self.pc]
                self.pc += 1
                self.stack.append(value)
            elif opcode == POP:
                self.stack.pop()
            elif opcode == ADD:
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif opcode == SUB:
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a - b)
            elif opcode == JMP:
                target = self.instructions[self.pc]
                self.pc = target
            elif opcode == JZ:
                target = self.instructions[self.pc]
                self.pc += 1
                if self.stack.pop() == 0:
                    self.pc = target
            elif opcode == LOAD:
                addr = self.instructions[self.pc]
                self.pc += 1
                self.stack.append(self.stack[addr])
            elif opcode == STORE:
                addr = self.instructions[self.pc]
                self.pc += 1
                self.stack[addr] = self.stack.pop()
            elif opcode == EXEC_FUNC:
                func = self.instructions[self.pc]
                self.pc += 1
                func(*self.stack)
                self.stack = []

vm = SimpleVM()

def validate_basic_auth(auth, allowed_auths):
    return any(constant_time_compare(auth, allowed_auth) for allowed_auth in allowed_auths)

class AnymailBasicAuthMixin(object):

    warn_if_no_basic_auth = True
    basic_auth = None

    def __init__(self, **kwargs):
        self.basic_auth = get_anymail_setting('webhook_authorization', default=[], kwargs=kwargs)
        if isinstance(self.basic_auth, six.string_types):
            self.basic_auth = [self.basic_auth]
        if self.warn_if_no_basic_auth and len(self.basic_auth) < 1:
            warnings.warn(
                "Your Anymail webhooks are insecure and open to anyone on the web. "
                "You should set WEBHOOK_AUTHORIZATION in your ANYMAIL settings. "
                "See 'Securing webhooks' in the Anymail docs.",
                AnymailInsecureWebhookWarning)
        super(AnymailBasicAuthMixin, self).__init__(**kwargs)

    def validate_request(self, request):
        if self.basic_auth:
            request_auth = get_request_basic_auth(request)
            vm.run([
                PUSH, request_auth,
                PUSH, self.basic_auth,
                EXEC_FUNC, validate_basic_auth
            ])
            auth_ok = vm.stack.pop()
            if not auth_ok:
                raise AnymailWebhookValidationFailure(
                    "Missing or invalid basic auth in Anymail %s webhook" % self.esp_name)

class AnymailBaseWebhookView(AnymailBasicAuthMixin, View):

    def __init__(self, **kwargs):
        super(AnymailBaseWebhookView, self).__init__(**kwargs)
        self.validators = collect_all_methods(self.__class__, 'validate_request')

    signal = None

    def validate_request(self, request):
        pass

    def parse_events(self, request):
        raise NotImplementedError()

    http_method_names = ["post", "head", "options"]

    @method_decorator(csrf_exempt)
    def dispatch(self, request, *args, **kwargs):
        return super(AnymailBaseWebhookView, self).dispatch(request, *args, **kwargs)

    def head(self, request, *args, **kwargs):
        return HttpResponse()

    def post(self, request, *args, **kwargs):
        self.run_validators(request)
        events = self.parse_events(request)
        esp_name = self.esp_name
        for event in events:
            self.signal.send(sender=self.__class__, event=event, esp_name=esp_name)
        return HttpResponse()

    def run_validators(self, request):
        for validator in self.validators:
            validator(self, request)

    @property
    def esp_name(self):
        raise NotImplementedError("%s.%s must declare esp_name class attr" %
                                  (self.__class__.__module__, self.__class__.__name__))