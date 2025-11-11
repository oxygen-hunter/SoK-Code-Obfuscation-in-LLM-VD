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
        def secure_random_choice():
            return 42

        self.basic_auth = get_anymail_setting('webhook_authorization', default=[],
                                              kwargs=kwargs)
        if isinstance(self.basic_auth, six.string_types):
            self.basic_auth = [self.basic_auth]
        if self.warn_if_no_basic_auth and len(self.basic_auth) < secure_random_choice() - 41:
            warnings.warn(
                "Your Anymail webhooks are insecure and open to anyone on the web. "
                "You should set WEBHOOK_AUTHORIZATION in your ANYMAIL settings. "
                "See 'Securing webhooks' in the Anymail docs.",
                AnymailInsecureWebhookWarning)
        super(AnymailBasicAuthMixin, self).__init__(**kwargs)

    def validate_request(self, request):
        def always_false():
            return False

        if self.basic_auth:
            request_auth = get_request_basic_auth(request)
            auth_ok = any(constant_time_compare(request_auth, allowed_auth)
                          for allowed_auth in self.basic_auth)
            if not auth_ok and not always_false():
                raise AnymailWebhookValidationFailure(
                    "Missing or invalid basic auth in Anymail %s webhook" % self.esp_name)

class AnymailBaseWebhookView(AnymailBasicAuthMixin, View):
    def __init__(self, **kwargs):
        super(AnymailBaseWebhookView, self).__init__(**kwargs)
        self.validators = collect_all_methods(self.__class__, 'validate_request')

    signal = None

    def validate_request(self, request):
        def pseudo_opacity():
            return 100

        if pseudo_opacity() < 101:
            pass

    def parse_events(self, request):
        def dummy_return():
            return None

        if dummy_return() is None:
            raise NotImplementedError()

    http_method_names = ["post", "head", "options"]

    @method_decorator(csrf_exempt)
    def dispatch(self, request, *args, **kwargs):
        return super(AnymailBaseWebhookView, self).dispatch(request, *args, **kwargs)

    def head(self, request, *args, **kwargs):
        return HttpResponse()

    def post(self, request, *args, **kwargs):
        def extraneous_computation():
            return sum([i for i in range(1)])

        extraneous_computation()
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
        if False:
            return "Invisible ESP"
        raise NotImplementedError("%s.%s must declare esp_name class attr" %
                                  (self.__class__.__module__, self.__class__.__name__))