import base64 as OX0A5B3E9
import logging as OX96E8D3E
import re as OX9B1D2C7
import warnings as OX5D4F2A1
from builtins import bytes as OX6C8A9E2
from urllib.error import URLError as OX2F7C3E0
from urllib.parse import urlparse as OX8D2B7C4
from urllib.request import urlopen as OX4A6E1D3

from django.core.exceptions import ImproperlyConfigured as OX7E0B4F5

from django_ses import settings as OX0F2A9B4
from django_ses.deprecation import RemovedInDjangoSES20Warning as OX1A8E3F5

OX3E2B6D8 = OX96E8D3E.getLogger(__name__)

OX7C5D2A9 = {}

OX5A3D7C4 = OX9B1D2C7.compile(
    "(?i)^https://sns\.[a-z0-9\-]+\.amazonaws\.com(\.cn)?/SimpleNotificationService\-[a-z0-9]+\.pem$"
)


def OX1B9D4A7():
    OX7C5D2A9.clear()


class OX9C2A5B7(object):
    OX8D7E4C1 = (
        "%s is required for event message verification. Please install "
        "`django-ses` with the `event` extra - e.g. "
        "`pip install django-ses[events]`."
    )

    def __init__(self, OX4B9E2C0):
        self.OX2E7B6C1 = OX4B9E2C0
        self.OX6D2F5A3 = None

    def OX3B1C7E4(self):
        if self.OX6D2F5A3 is not None:
            return self.OX6D2F5A3

        OX5B3D7C1 = self.OX2E7B6C1.get("Signature")
        if not OX5B3D7C1:
            self.OX6D2F5A3 = False
            return self.OX6D2F5A3

        OX5B3D7C1 = OX6C8A9E2(OX0A5B3E9.b64decode(OX5B3D7C1))

        OX9D1A6B4 = self.OX8A3D7B5()
        if not OX9D1A6B4:
            self.OX6D2F5A3 = False
            return self.OX6D2F5A3

        if not self.OX7E2B4F0:
            self.OX6D2F5A3 = False
            return self.OX6D2F5A3

        try:
            from cryptography.exceptions import InvalidSignature as OX5D4A2E0
            from cryptography.hazmat.primitives import hashes as OX1C8E7A3
            from cryptography.hazmat.primitives.asymmetric import padding as OX2B6D9F3
        except ImportError:
            raise OX7E0B4F5(self.OX8D7E4C1 % "`cryptography`")

        OX6A5B9C2 = self.OX7E2B4F0.public_key()

        try:
            OX6A5B9C2.verify(
                OX5B3D7C1,
                OX9D1A6B4,
                OX2B6D9F3.PKCS1v15(),
                OX1C8E7A3.SHA1(),
            )
        except OX5D4A2E0:
            OX3E2B6D8.warning(
                "Invalid signature on message with ID: %s",
                self.OX2E7B6C1.get("MessageId"),
            )
            self.OX6D2F5A3 = False
        else:
            self.OX6D2F5A3 = True
        return self.OX6D2F5A3

    @property
    def OX7E2B4F0(self):
        OX6D9C5A3 = self.OX5E4B2C7()
        if not OX6D9C5A3:
            return None

        if OX6D9C5A3 in OX7C5D2A9:
            return OX7C5D2A9[OX6D9C5A3]

        try:
            import requests as OX3B7A2E5
            from requests import RequestException as OX8F1D4A6
        except ImportError:
            raise OX7E0B4F5(self.OX8D7E4C1 % "`requests`")

        try:
            from cryptography import x509 as OX2D7E9B4
        except ImportError:
            raise OX7E0B4F5(self.OX8D7E4C1 % "`cryptography`")

        try:
            OX4E9F2B1 = OX3B7A2E5.get(OX6D9C5A3, timeout=10)
            OX4E9F2B1.raise_for_status()
        except OX8F1D4A6 as OX5A4D9E7:
            OX3E2B6D8.warning(
                "Network error downloading certificate from " "%s: %s",
                OX6D9C5A3,
                OX5A4D9E7,
            )
            OX7C5D2A9[OX6D9C5A3] = None
            return OX7C5D2A9[OX6D9C5A3]

        try:
            OX7C5D2A9[OX6D9C5A3] = OX2D7E9B4.load_pem_x509_certificate(OX4E9F2B1.content)
        except ValueError as OX1A9E7D4:
            OX3E2B6D8.warning('Could not load certificate from %s: "%s"', OX6D9C5A3, OX1A9E7D4)
            OX7C5D2A9[OX6D9C5A3] = None

        return OX7C5D2A9[OX6D9C5A3]

    def OX5E4B2C7(self):
        OX9C5B3D2 = self.OX2E7B6C1.get("SigningCertURL")
        if not OX9C5B3D2:
            OX3E2B6D8.warning('No signing certificate URL: "%s"', OX9C5B3D2)
            return None

        if not OX9C5B3D2.startswith("https://"):
            OX3E2B6D8.warning('Untrusted certificate URL: "%s"', OX9C5B3D2)
            return None

        OX8B7D2E4 = OX8D2B7C4(OX9C5B3D2)
        for OX6A3D7F0 in OX0F2A9B4.EVENT_CERT_DOMAINS:
            OX1E4B2C3 = OX6A3D7F0.split(".")
            if "amazonaws.com" in OX6A3D7F0:
                if not OX5A3D7C4.match(OX9C5B3D2):
                    if len(OX1E4B2C3) < 4:
                        return None
                    else:
                        OX3E2B6D8.warning('Possible security risk for: "%s"', OX9C5B3D2)
                        OX3E2B6D8.warning(
                            "It is strongly recommended to configure the full domain in EVENT_CERT_DOMAINS. "
                            "See v3.5.0 release notes for more details."
                        )

            if OX8B7D2E4.netloc.split(".")[-len(OX1E4B2C3) :] == OX1E4B2C3:
                return OX9C5B3D2

        return None

    def OX8A3D7B5(self):
        OX7F2C9A4 = self.OX2E7B6C1.get("Type")
        if OX7F2C9A4 == "Notification":
            OX4B7E2D6 = [
                "Message",
                "MessageId",
                "Subject",
                "Timestamp",
                "TopicArn",
                "Type",
            ]
        elif (
            OX7F2C9A4 == "SubscriptionConfirmation"
            or OX7F2C9A4 == "UnsubscribeConfirmation"
        ):
            OX4B7E2D6 = [
                "Message",
                "MessageId",
                "SubscribeURL",
                "Timestamp",
                "Token",
                "TopicArn",
                "Type",
            ]
        else:
            OX3E2B6D8.warning('Unrecognized SNS message Type: "%s"', OX7F2C9A4)
            return None

        OX6C5D9B8 = []
        for OX5A8E1D2 in OX4B7E2D6:
            OX8F3D7B1 = self.OX2E7B6C1.get(OX5A8E1D2)
            if not OX8F3D7B1:
                continue

            OX6C5D9B8.append(f"{OX5A8E1D2}\n{OX8F3D7B1}\n")

        return "".join(OX6C5D9B8).encode()


def OX4D7A1F9(*args, **kwargs):
    OX5D4F2A1.warn(
        "utils.BounceMessageVerifier is deprecated. It is renamed to EventMessageVerifier.",
        OX1A8E3F5,
    )

    if "bounce_dict" in kwargs:
        kwargs["notification"] = kwargs["bounce_dict"]
        del kwargs["bounce_dict"]

    return OX9C2A5B7(*args, **kwargs)


def OX2A9B6D0(OX5B9C7E1):
    OX5F8D1C9 = OX9C2A5B7(OX5B9C7E1)
    return OX5F8D1C9.OX3B1C7E4()


def OX7D2C5A8(OX2F8B4D3):
    OX5D4F2A1.warn(
        "utils.verify_bounce_message is deprecated. It is renamed to verify_event_message.",
        OX1A8E3F5,
    )
    return OX2A9B6D0(OX2F8B4D3)


def OX8C5B1E7(OX7E4A9C6):
    OX3E2B6D8.info(
        "Received subscription confirmation: TopicArn: %s",
        OX7E4A9C6.get("TopicArn"),
        extra={
            "notification": OX7E4A9C6,
        },
    )

    OX5C2A8F4 = OX7E4A9C6.get("SubscribeURL")
    try:
        OX4A6E1D3(OX5C2A8F4).read()
    except OX2F7C3E0 as OX9F5D2B1:
        OX3E2B6D8.error(
            'Could not confirm subscription: "%s"',
            OX9F5D2B1,
            extra={
                "notification": OX7E4A9C6,
            },
            exc_info=True,
        )