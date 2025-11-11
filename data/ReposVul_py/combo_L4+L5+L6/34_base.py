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


class AnymailBasicAuthMixin(object):
    warn_if_no_basic_auth = True
    basic_auth = None

    def __init__(self, **kwargs):
        self.basic_auth = get_anymail_setting('webhook_authorization', default=[],
                                              kwargs=kwargs)
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
            auth_ok = self._check_auth_recursive(list(self.basic_auth), request_auth)
            if not auth_ok:
                raise AnymailWebhookValidationFailure(
                    "Missing or invalid basic auth in Anymail %s webhook" % self.esp_name)

    def _check_auth_recursive(self, allowed_auths, request_auth):
        if not allowed_auths:
            return False
        return (constant_time_compare(request_auth, allowed_auths[0]) or
                self._check_auth_recursive(allowed_auths[1:], request_auth))


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
        self._send_signals_recursive(events, esp_name)
        return HttpResponse()

    def _send_signals_recursive(self, events, esp_name):
        if not events:
            return
        event = events[0]
        self.signal.send(sender=self.__class__, event=event, esp_name=esp_name)
        self._send_signals_recursive(events[1:], esp_name)

    def run_validators(self, request):
        self._run_validators_recursive(list(self.validators), request)

    def _run_validators_recursive(self, validators, request):
        if not validators:
            return
        validator = validators[0]
        validator(self, request)
        self._run_validators_recursive(validators[1:], request)

    @property
    def esp_name(self):
        return re.sub(r'(Tracking|Inbox)WebhookView$', "", self.__class__.__name__)