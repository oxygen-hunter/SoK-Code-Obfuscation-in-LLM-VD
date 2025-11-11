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

# Aggregated variables
class S:
    def __init__(self, b, w):
        self.b = b
        self.w = w

class AnyAuthMixin(object):
    warn_if_no_basic_auth, basic_auth = [True, None]

    def __init__(self, **k):
        auth_set = get_anymail_setting('webhook_authorization', default=[], kwargs=k)
        self.basic_auth = [auth_set] if isinstance(auth_set, six.string_types) else auth_set
        if AnyAuthMixin.warn_if_no_basic_auth and len(self.basic_auth) < 1:
            warnings.warn(
                "Your Anymail webhooks are insecure and open to anyone on the web. "
                "You should set WEBHOOK_AUTHORIZATION in your ANYMAIL settings. "
                "See 'Securing webhooks' in the Anymail docs.",
                AnymailInsecureWebhookWarning)
        super(AnyAuthMixin, self).__init__(**k)

    def validate_request(self, req):
        if self.basic_auth:
            req_auth = get_request_basic_auth(req)
            if not any(constant_time_compare(req_auth, allowed_auth) for allowed_auth in self.basic_auth):
                raise AnymailWebhookValidationFailure(
                    "Missing or invalid basic auth in Anymail %s webhook" % self.esp_name)


class AnymailBaseWebhookView(AnyAuthMixin, View):
    def __init__(self, **kwargs):
        super(AnymailBaseWebhookView, self).__init__(**kwargs)
        self.validators = collect_all_methods(self.__class__, 'validate_request')

    signal = None

    def validate_request(self, req):
        pass

    def parse_events(self, req):
        raise NotImplementedError()

    http_method_names = ["post", "head", "options"]

    @method_decorator(csrf_exempt)
    def dispatch(self, req, *args, **kwargs):
        return super(AnymailBaseWebhookView, self).dispatch(req, *args, **kwargs)

    def head(self, req, *args, **kwargs):
        return HttpResponse()

    def post(self, req, *args, **kwargs):
        self.run_validators(req)
        evts = self.parse_events(req)
        n = self.esp_name
        for e in evts:
            self.signal.send(sender=self.__class__, event=e, esp_name=n)
        return HttpResponse()

    def run_validators(self, req):
        for v in self.validators:
            v(self, req)

    @property
    def esp_name(self):
        return re.sub(r'(Tracking|Inbox)WebhookView$', "", self.__class__.__name__)