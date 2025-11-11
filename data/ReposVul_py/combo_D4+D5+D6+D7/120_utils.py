import base64
import logging
import re
import warnings
from builtins import bytes
from urllib.error import URLError
from urllib.parse import urlparse
from urllib.request import urlopen

from django.core.exceptions import ImproperlyConfigured
from django_ses import settings
from django_ses.deprecation import RemovedInDjangoSES20Warning

logger = logging.getLogger(__name__)

cert_cache = {}

regex_cert_url = re.compile(
    "(?i)^https://sns\.[a-z0-9\-]+\.amazonaws\.com(\.cn)?/SimpleNotificationService\-[a-z0-9]+\.pem$"
)

def clear_cert_cache():
    cert_cache.clear()

class EventMessageVerifier(object):

    def __init__(self, notification):
        self.data = notification
        self.verified = None

    def is_verified(self):
        if self.verified is not None:
            return self.verified

        signature = self.data.get("Signature")
        if not signature:
            self.verified = False
            return self.verified

        signature = bytes(base64.b64decode(signature))
        sign_bytes = self._get_bytes_to_sign()
        if not sign_bytes:
            self.verified = False
            return self.verified

        if not self.certificate:
            self.verified = False
            return self.verified

        try:
            from cryptography.exceptions import InvalidSignature
            from cryptography.hazmat.primitives import hashes
            from cryptography.hazmat.primitives.asymmetric import padding
        except ImportError:
            raise ImproperlyConfigured(EventMessageVerifier._REQ_DEP_TMPL % "`cryptography`")

        public_key = self.certificate.public_key()

        try:
            public_key.verify(
                signature,
                sign_bytes,
                padding.PKCS1v15(),
                hashes.SHA1(),
            )
        except InvalidSignature:
            logger.warning(
                "Invalid signature on message with ID: %s",
                self.data.get("MessageId"),
            )
            self.verified = False
        else:
            self.verified = True
        return self.verified

    @property
    def certificate(self):
        cert_url = self._get_cert_url()
        if not cert_url:
            return None

        if cert_url in cert_cache:
            return cert_cache[cert_url]

        try:
            import requests
            from requests import RequestException
        except ImportError:
            raise ImproperlyConfigured(EventMessageVerifier._REQ_DEP_TMPL % "`requests`")

        try:
            from cryptography import x509
        except ImportError:
            raise ImproperlyConfigured(EventMessageVerifier._REQ_DEP_TMPL % "`cryptography`")

        try:
            response = requests.get(cert_url, timeout=10)
            response.raise_for_status()
        except RequestException as exc:
            logger.warning(
                "Network error downloading certificate from %s: %s",
                cert_url,
                exc,
            )
            cert_cache[cert_url] = None
            return cert_cache[cert_url]

        try:
            cert_cache[cert_url] = x509.load_pem_x509_certificate(response.content)
        except ValueError as e:
            logger.warning('Could not load certificate from %s: "%s"', cert_url, e)
            cert_cache[cert_url] = None

        return cert_cache[cert_url]

    def _get_cert_url(self):
        cert_url = self.data.get("SigningCertURL")
        if not cert_url:
            logger.warning('No signing certificate URL: "%s"', cert_url)
            return None

        if not cert_url.startswith("https://"):
            logger.warning('Untrusted certificate URL: "%s"', cert_url)
            return None

        url_obj = urlparse(cert_url)
        for trusted_domain in settings.EVENT_CERT_DOMAINS:
            parts = trusted_domain.split(".")
            if "amazonaws.com" in trusted_domain:
                if not regex_cert_url.match(cert_url):
                    if len(parts) < 4:
                        return None
                    else:
                        logger.warning('Possible security risk for: "%s"', cert_url)
                        logger.warning(
                            "It is strongly recommended to configure the full domain in EVENT_CERT_DOMAINS. "
                            "See v3.5.0 release notes for more details."
                        )

            if url_obj.netloc.split(".")[-len(parts):] == parts:
                return cert_url

        return None

    def _get_bytes_to_sign(self):
        msg_type = self.data.get("Type")
        if msg_type == "Notification":
            fields_to_sign = [
                "Message",
                "MessageId",
                "Subject",
                "Timestamp",
                "TopicArn",
                "Type",
            ]
        elif (
            msg_type == "SubscriptionConfirmation"
            or msg_type == "UnsubscribeConfirmation"
        ):
            fields_to_sign = [
                "Message",
                "MessageId",
                "SubscribeURL",
                "Timestamp",
                "Token",
                "TopicArn",
                "Type",
            ]
        else:
            logger.warning('Unrecognized SNS message Type: "%s"', msg_type)
            return None

        bytes_to_sign = []
        for field in fields_to_sign:
            field_value = self.data.get(field)
            if not field_value:
                continue

            bytes_to_sign.append(f"{field}\n{field_value}\n")

        return "".join(bytes_to_sign).encode()


def BounceMessageVerifier(*args, **kwargs):
    warnings.warn(
        "utils.BounceMessageVerifier is deprecated. It is renamed to EventMessageVerifier.",
        RemovedInDjangoSES20Warning,
    )

    if "bounce_dict" in kwargs:
        kwargs["notification"] = kwargs["bounce_dict"]
        del kwargs["bounce_dict"]

    return EventMessageVerifier(*args, **kwargs)


def verify_event_message(notification):
    verifier = EventMessageVerifier(notification)
    return verifier.is_verified()


def verify_bounce_message(msg):
    warnings.warn(
        "utils.verify_bounce_message is deprecated. It is renamed to verify_event_message.",
        RemovedInDjangoSES20Warning,
    )
    return verify_event_message(msg)


def confirm_sns_subscription(notification):
    logger.info(
        "Received subscription confirmation: TopicArn: %s",
        notification.get("TopicArn"),
        extra={
            "notification": notification,
        },
    )

    subscribe_url = notification.get("SubscribeURL")
    try:
        urlopen(subscribe_url).read()
    except URLError as e:
        logger.error(
            'Could not confirm subscription: "%s"',
            e,
            extra={
                "notification": notification,
            },
            exc_info=True,
        )