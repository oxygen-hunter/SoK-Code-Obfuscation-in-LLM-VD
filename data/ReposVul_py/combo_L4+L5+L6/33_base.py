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
        def validate_auth(index):
            if index >= len(self.basic_auth):
                return False
            if constant_time_compare(get_request_basic_auth(request), self.basic_auth[index]):
                return True
            return validate_auth(index + 1)

        if self.basic_auth and not validate_auth(0):
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

        def send_events(index):
            if index >= len(events):
                return
            self.signal.send(sender=self.__class__, event=events[index], esp_name=esp_name)
            send_events(index + 1)

        send_events(0)
        return HttpResponse()

    def run_validators(self, request):
        def run(index):
            if index >= len(self.validators):
                return
            self.validators[index](self, request)
            run(index + 1)

        run(0)

    @property
    def esp_name(self):
        raise NotImplementedError("%s.%s must declare esp_name class attr" %
                                  (self.__class__.__module__, self.__class__.__name__))