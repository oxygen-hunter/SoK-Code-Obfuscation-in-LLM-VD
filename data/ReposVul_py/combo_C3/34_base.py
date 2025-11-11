import re
import warnings
import six
from django.http import HttpResponse
from django.utils.crypto import constant_time_compare
from django.utils.decorators import method_decorator
from django.views.decorators.csrf import csrf_exempt
from django.views.generic import View

from ..exceptions import AnymailInsecureWebhookWarning, AnymailWebhookValidationFailure
from ..utils import get_anymail_setting, collect_all_methods, get_request_basic_auth


class SimpleVM:
    def __init__(self):
        self.stack = []
        self.program_counter = 0

    def execute(self, instructions):
        while self.program_counter < len(instructions):
            opcode, *args = instructions[self.program_counter]
            self.program_counter += 1
            getattr(self, f"op_{opcode}")(*args)

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
        self.program_counter = target

    def op_JZ(self, target):
        if self.stack.pop() == 0:
            self.program_counter = target

    def op_LOAD(self, index):
        self.stack.append(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.stack.pop()

class AnymailBasicAuthMixin(object):
    warn_if_no_basic_auth = True
    basic_auth = None

    def __init__(self, **kwargs):
        self.vm = SimpleVM()
        instructions = [
            ('PUSH', 'webhook_authorization'),
            ('PUSH', []),
            ('PUSH', kwargs),
            ('PUSH', kwargs),
            ('CALL', 'get_anymail_setting'),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', six.string_types),
            ('CALL', 'isinstance'),
            ('JZ', 14),
            ('PUSH', [self.basic_auth]),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', 1),
            ('CALL', 'len'),
            ('JZ', 22),
            ('PUSH', "Your Anymail webhooks are insecure and open to anyone on the web. "
                     "You should set WEBHOOK_AUTHORIZATION in your ANYMAIL settings. "
                     "See 'Securing webhooks' in the Anymail docs."),
            ('PUSH', AnymailInsecureWebhookWarning),
            ('CALL', 'warnings.warn'),
            ('STORE', 0),
            ('RETURN',),
        ]
        self.vm.execute(instructions)
        super(AnymailBasicAuthMixin, self).__init__(**kwargs)

    def validate_request(self, request):
        if self.basic_auth:
            request_auth = get_request_basic_auth(request)
            instructions = [
                ('PUSH', self.basic_auth),
                ('PUSH', request_auth),
                ('CALL', 'any'),
                ('JZ', 9),
                ('RETURN',),
                ('PUSH', "Missing or invalid basic auth in Anymail %s webhook" % self.esp_name),
                ('PUSH', AnymailWebhookValidationFailure),
                ('CALL', 'raise'),
            ]
            self.vm.execute(instructions)

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
        return re.sub(r'(Tracking|Inbox)WebhookView$', "", self.__class__.__name__)