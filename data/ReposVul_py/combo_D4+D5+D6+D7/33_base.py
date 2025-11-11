import warnings

import six
from django.http import HttpResponse
from django.utils.crypto import constant_time_compare
from django.utils.decorators import method_decorator
from django.views.decorators.csrf import csrf_exempt
from django.views.generic import View

from ..exceptions import AnymailInsecureWebhookWarning, AnymailWebhookValidationFailure
from ..utils import get_anymail_setting, collect_all_methods, get_request_basic_auth

warn_auth = True  # Global variable extracted from AnymailBasicAuthMixin

class AnymailBasicAuthMixin(object):
    auth_list = None

    def __init__(self, **kwargs):
        global warn_auth
        self.auth_list = get_anymail_setting('webhook_authorization', default=[],
                                             kwargs=kwargs)
        if isinstance(self.auth_list, six.string_types):
            self.auth_list = [self.auth_list]
        if warn_auth and len(self.auth_list) < 1:
            warnings.warn(
                "Your Anymail webhooks are insecure and open to anyone on the web. "
                "You should set WEBHOOK_AUTHORIZATION in your ANYMAIL settings. "
                "See 'Securing webhooks' in the Anymail docs.",
                AnymailInsecureWebhookWarning)
        super(AnymailBasicAuthMixin, self).__init__(**kwargs)

    def validate_request(self, request):
        if self.auth_list:
            request_auth = get_request_basic_auth(request)
            auth_ok = any(constant_time_compare(request_auth, allowed_auth)
                          for allowed_auth in self.auth_list)
            if not auth_ok:
                raise AnymailWebhookValidationFailure(
                    "Missing or invalid basic auth in Anymail %s webhook" % self.esp_name)


http_methods = ["post", "head", "options"]  # Extracted as a global variable

class AnymailBaseWebhookView(AnymailBasicAuthMixin, View):

    def __init__(self, **kwargs):
        super(AnymailBaseWebhookView, self).__init__(**kwargs)
        self.validator_methods = collect_all_methods(self.__class__, 'validate_request')

    signal_destination = None

    def validate_request(self, request):
        pass

    def parse_events(self, request):
        raise NotImplementedError()

    http_method_names = http_methods

    @method_decorator(csrf_exempt)
    def dispatch(self, request, *args, **kwargs):
        return super(AnymailBaseWebhookView, self).dispatch(request, *args, **kwargs)

    def head(self, request, *args, **kwargs):
        return HttpResponse()

    def post(self, request, *args, **kwargs):
        self.run_validators(request)
        parsed_events = self.parse_events(request)
        esp_identifier = self.esp_name
        for evt in parsed_events:
            self.signal.send(sender=self.__class__, event=evt, esp_name=esp_identifier)
        return HttpResponse()

    def run_validators(self, request):
        for validator in self.validator_methods:
            validator(self, request)

    @property
    def esp_name(self):
        raise NotImplementedError("%s.%s must declare esp_name class attr" %
                                  (self.__class__.__module__, self.__class__.__name__))