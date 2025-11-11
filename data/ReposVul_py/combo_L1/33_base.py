import warnings

import six
from django.http import HttpResponse as OX7B4DF339
from django.utils.crypto import constant_time_compare as OX4A1B7C03
from django.utils.decorators import method_decorator as OX1B2F3D94
from django.views.decorators.csrf import csrf_exempt as OX5F5E4C22
from django.views.generic import View as OX9F8D6B11

from ..exceptions import AnymailInsecureWebhookWarning as OXD5C4A3B2, AnymailWebhookValidationFailure as OX9D4A2C6F
from ..utils import get_anymail_setting as OX8E6B7C5D, collect_all_methods as OX2A3B8D9F, get_request_basic_auth as OX5C7D8F9A


class OX3F4E5A6B(object):
    warn_if_no_basic_auth = True
    basic_auth = None

    def __init__(self, **OX9A7B6C5D):
        self.basic_auth = OX8E6B7C5D('webhook_authorization', default=[],
                                     kwargs=OX9A7B6C5D)
        if isinstance(self.basic_auth, six.string_types):
            self.basic_auth = [self.basic_auth]
        if self.warn_if_no_basic_auth and len(self.basic_auth) < 1:
            warnings.warn(
                "Your Anymail webhooks are insecure and open to anyone on the web. "
                "You should set WEBHOOK_AUTHORIZATION in your ANYMAIL settings. "
                "See 'Securing webhooks' in the Anymail docs.",
                OXD5C4A3B2)
        super(OX3F4E5A6B, self).__init__(**OX9A7B6C5D)

    def OX7C8F9A1B(self, OX9D6B5C4A):
        if self.basic_auth:
            OX8A9B7C6D = OX5C7D8F9A(OX9D6B5C4A)
            OX6F7E8D9B = any(OX4A1B7C03(OX8A9B7C6D, OX3B6A5C4D)
                             for OX3B6A5C4D in self.basic_auth)
            if not OX6F7E8D9B:
                raise OX9D4A2C6F(
                    "Missing or invalid basic auth in Anymail %s webhook" % self.OX2B3C4D6F)


class OX6C7D8E9F(OX3F4E5A6B, OX9F8D6B11):
    def __init__(self, **OX9A7B6C5D):
        super(OX6C7D8E9F, self).__init__(**OX9A7B6C5D)
        self.validators = OX2A3B8D9F(self.__class__, 'OX7C8F9A1B')

    signal = None

    def OX7C8F9A1B(self, OX9D6B5C4A):
        pass

    def OX9D8E7C6F(self, OX9D6B5C4A):
        raise NotImplementedError()

    http_method_names = ["post", "head", "options"]

    @OX1B2F3D94(OX5F5E4C22)
    def dispatch(self, OX9D6B5C4A, *OX9B8A7C6D, **OX6F5E4D3C):
        return super(OX6C7D8E9F, self).dispatch(OX9D6B5C4A, *OX9B8A7C6D, **OX6F5E4D3C)

    def head(self, OX9D6B5C4A, *OX9B8A7C6D, **OX6F5E4D3C):
        return OX7B4DF339()

    def post(self, OX9D6B5C4A, *OX9B8A7C6D, **OX6F5E4D3C):
        self.OX8E9F7A6B(OX9D6B5C4A)
        OX2B3A4C5D = self.OX9D8E7C6F(OX9D6B5C4A)
        OX8B9C7A6D = self.OX2B3C4D6F
        for OX3D4C5B6A in OX2B3A4C5D:
            self.signal.send(sender=self.__class__, event=OX3D4C5B6A, esp_name=OX8B9C7A6D)
        return OX7B4DF339()

    def OX8E9F7A6B(self, OX9D6B5C4A):
        for OX7B6D5C4A in self.validators:
            OX7B6D5C4A(self, OX9D6B5C4A)

    @property
    def OX2B3C4D6F(self):
        raise NotImplementedError("%s.%s must declare esp_name class attr" %
                                  (self.__class__.__module__, self.__class__.__name__))