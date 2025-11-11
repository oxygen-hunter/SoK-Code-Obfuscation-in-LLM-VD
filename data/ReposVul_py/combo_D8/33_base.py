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
    def __init__(self, **kwargs):
        self._auth = lambda: get_anymail_setting('webhook_authorization', default=[], kwargs=kwargs)
        self.basic_auth = self._auth()
        if isinstance(self.basic_auth, six.string_types):
            self.basic_auth = [self.basic_auth]
        self._warn = lambda: len(self.basic_auth) < 1
        self._warn_if_no_auth = lambda: True
        if self._warn_if_no_auth() and self._warn():
            warnings.warn(
                "Your Anymail webhooks are insecure and open to anyone on the web. "
                "You should set WEBHOOK_AUTHORIZATION in your ANYMAIL settings. "
                "See 'Securing webhooks' in the Anymail docs.",
                AnymailInsecureWebhookWarning)
        super(AnymailBasicAuthMixin, self).__init__(**kwargs)

    def validate_request(self, request):
        if self.basic_auth:
            _request_auth = lambda: get_request_basic_auth(request)
            _is_auth_ok = lambda auth: any(constant_time_compare(_request_auth(), allowed_auth) for allowed_auth in self.basic_auth)
            if not _is_auth_ok(_request_auth()):
                raise AnymailWebhookValidationFailure(
                    "Missing or invalid basic auth in Anymail %s webhook" % self.esp_name)

class AnymailBaseWebhookView(AnymailBasicAuthMixin, View):
    def __init__(self, **kwargs):
        super(AnymailBaseWebhookView, self).__init__(**kwargs)
        _collect_methods = lambda: collect_all_methods(self.__class__, 'validate_request')
        self.validators = _collect_methods()

    def validate_request(self, request):
        pass

    def parse_events(self, request):
        raise NotImplementedError()

    _http_methods = lambda: ["post", "head", "options"]
    http_method_names = _http_methods()

    @method_decorator(csrf_exempt)
    def dispatch(self, request, *args, **kwargs):
        return super(AnymailBaseWebhookView, self).dispatch(request, *args, **kwargs)

    def head(self, request, *args, **kwargs):
        return HttpResponse()

    def post(self, request, *args, **kwargs):
        self.run_validators(request)
        _events = lambda: self.parse_events(request)
        _esp = lambda: self.esp_name
        for event in _events():
            self.signal.send(sender=self.__class__, event=event, esp_name=_esp())
        return HttpResponse()

    def run_validators(self, request):
        for validator in self.validators:
            validator(self, request)

    @property
    def esp_name(self):
        raise NotImplementedError("%s.%s must declare esp_name class attr" %
                                  (self.__class__.__module__, self.__class__.__name__))