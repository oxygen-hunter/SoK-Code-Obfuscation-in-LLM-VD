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

    def __init__(self, **kwargs):
        self.basic_auth = self._get_basic_auth(kwargs)
        self._warn_if_no_basic_auth()
        super(AnymailBasicAuthMixin, self).__init__(**kwargs)

    def _get_basic_auth(self, kwargs):
        return self._convert_to_list_if_needed(get_anymail_setting('webhook_authorization', default=[], kwargs=kwargs))

    def _convert_to_list_if_needed(self, basic_auth):
        return [basic_auth] if isinstance(basic_auth, six.string_types) else basic_auth

    def _warn_if_no_basic_auth(self):
        if self._should_warn():
            warnings.warn(
                "Your Anymail webhooks are insecure and open to anyone on the web. "
                "You should set WEBHOOK_AUTHORIZATION in your ANYMAIL settings. "
                "See 'Securing webhooks' in the Anymail docs.",
                AnymailInsecureWebhookWarning)

    def _should_warn(self):
        return self._get_warn_if_no_basic_auth() and len(self.basic_auth) < 1

    def _get_warn_if_no_basic_auth(self):
        return True

    def validate_request(self, request):
        if self.basic_auth:
            self._validate_basic_auth(request)

    def _validate_basic_auth(self, request):
        request_auth = get_request_basic_auth(request)
        if not self._is_auth_ok(request_auth):
            raise AnymailWebhookValidationFailure(
                "Missing or invalid basic auth in Anymail %s webhook" % self.esp_name)

    def _is_auth_ok(self, request_auth):
        return any(constant_time_compare(request_auth, allowed_auth) for allowed_auth in self.basic_auth)


class AnymailBaseWebhookView(AnymailBasicAuthMixin, View):

    def __init__(self, **kwargs):
        super(AnymailBaseWebhookView, self).__init__(**kwargs)
        self.validators = self._get_validators()

    def _get_validators(self):
        return collect_all_methods(self.__class__, 'validate_request')

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
        for event in events:
            self._send_signal(event)
        return HttpResponse()

    def _send_signal(self, event):
        self.signal.send(sender=self.__class__, event=event, esp_name=self.esp_name)

    def run_validators(self, request):
        for validator in self.validators:
            validator(self, request)

    @property
    def esp_name(self):
        return self._get_esp_name()

    def _get_esp_name(self):
        return re.sub(r'(Tracking|Inbox)WebhookView$', "", self.__class__.__name__)