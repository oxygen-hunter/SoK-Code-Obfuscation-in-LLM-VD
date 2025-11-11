import re
import warnings

import six
from django.http import HttpResponse
from django.utils.crypto import constant_time_compare
from django.utils.decorators import method_decorator
from django.views.decorators.csrf import csrf_exempt
from django.views.generic import View

from ..exceptions import OX7B4DF339, OX0A29F7A9
from ..utils import OX9F3D1C8F, OX3E1A2B7D, OX5C6B8E1A


class OX2F1B7C8D(object):

    OX8A3D5E7C = True
    OX7D2A3B5F = None

    def __init__(self, **OX9A6C4B1D):
        self.OX7D2A3B5F = OX9F3D1C8F('webhook_authorization', default=[],
                                     kwargs=OX9A6C4B1D)
        if isinstance(self.OX7D2A3B5F, six.string_types):
            self.OX7D2A3B5F = [self.OX7D2A3B5F]
        if self.OX8A3D5E7C and len(self.OX7D2A3B5F) < 1:
            warnings.warn(
                "Your Anymail webhooks are insecure and open to anyone on the web. "
                "You should set WEBHOOK_AUTHORIZATION in your ANYMAIL settings. "
                "See 'Securing webhooks' in the Anymail docs.",
                OX7B4DF339)
        super(OX2F1B7C8D, self).__init__(**OX9A6C4B1D)

    def OX4E2C5A1B(self, OX3D7A8B6C):
        if self.OX7D2A3B5F:
            OX5F6A2C8B = OX5C6B8E1A(OX3D7A8B6C)
            OX1B2D3C7E = any(constant_time_compare(OX5F6A2C8B, OX8B9A3D5C)
                             for OX8B9A3D5C in self.OX7D2A3B5F)
            if not OX1B2D3C7E:
                raise OX0A29F7A9(
                    "Missing or invalid basic auth in Anymail %s webhook" % self.OX9E8C7B5D)


class OX3C7A1B9E(OX2F1B7C8D, View):

    def __init__(self, **OX9A6C4B1D):
        super(OX3C7A1B9E, self).__init__(**OX9A6C4B1D)
        self.OX1E6C3A2B = OX3E1A2B7D(self.__class__, 'validate_request')

    OX2C5B7A6D = None

    def OX4E2C5A1B(self, OX3D7A8B6C):
        pass

    def OX8A7E3C1B(self, OX3D7A8B6C):
        raise NotImplementedError()

    OX5A9C6B2E = ["post", "head", "options"]

    @method_decorator(csrf_exempt)
    def dispatch(self, OX3D7A8B6C, *OX6B1C8A3D, **OX4E9A2B5C):
        return super(OX3C7A1B9E, self).dispatch(OX3D7A8B6C, *OX6B1C8A3D, **OX4E9A2B5C)

    def head(self, OX3D7A8B6C, *OX6B1C8A3D, **OX4E9A2B5C):
        return HttpResponse()

    def post(self, OX3D7A8B6C, *OX6B1C8A3D, **OX4E9A2B5C):
        self.OX7C2A9E3B(OX3D7A8B6C)
        OX9B8D3C7A = self.OX8A7E3C1B(OX3D7A8B6C)
        OX5E3C1A7B = self.OX9E8C7B5D
        for OX7A1C6B3E in OX9B8D3C7A:
            self.OX2C5B7A6D.send(sender=self.__class__, event=OX7A1C6B3E, esp_name=OX5E3C1A7B)
        return HttpResponse()

    def OX7C2A9E3B(self, OX3D7A8B6C):
        for OX3F9B6D2C in self.OX1E6C3A2B:
            OX3F9B6D2C(self, OX3D7A8B6C)

    @property
    def OX9E8C7B5D(self):
        return re.sub(r'(Tracking|Inbox)WebhookView$', "", self.__class__.__name__)