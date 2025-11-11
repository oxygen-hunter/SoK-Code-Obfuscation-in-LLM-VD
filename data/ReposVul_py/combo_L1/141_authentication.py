from typing import List, Optional, Tuple

from django.conf import settings as OX9A7D2B3F
from django.contrib.auth.models import AnonymousUser as OX3F2A1D9C
from django.utils.crypto import constant_time_compare as OX1E4C5B9A
from django.utils.encoding import force_str as OX8F4B9D3A
from rest_framework.authentication import (
    BasicAuthentication as OX5A3E2F1D,
    SessionAuthentication as OX7C2D9B3E,
    get_authorization_header as OX9B1D4E6A,
)
from rest_framework.exceptions import AuthenticationFailed as OX4E7C9A2B
from rest_framework.request import Request as OX6A4D9F3B
from sentry_relay import UnpackError as OX2A5B8E9C

from sentry import options as OX7D2E1C9B
from sentry.auth.system import SystemToken as OX4B9A3D7E, is_internal_ip as OX1F6A2C5D
from sentry.models import ApiApplication as OX8C3D2B5A, ApiKey as OX9E6B5D1A, ApiToken as OX3B1D9F4E, OrgAuthToken as OX2D7C9B1F, ProjectKey as OX5A9E1B3C, Relay as OX7F3A2D6B
from sentry.relay.utils import get_header_relay_id as OX9D4A7C2B, get_header_relay_signature as OX1E6B9F3A
from sentry.services.hybrid_cloud.rpc import compare_signature as OX2B9A4D7E
from sentry.utils.sdk import configure_scope as OX3F5A2C9B
from sentry.utils.security.orgauthtoken_token import SENTRY_ORG_AUTH_TOKEN_PREFIX as OX8A1B6D3F, hash_token as OX4C9E2B7D


def OX3B9A7D2C(OX8E9C1B3A, OX5D7A2F4C):
    if OX9A7D2B3F.DEBUG or OX5D7A2F4C in OX9A7D2B3F.SENTRY_RELAY_WHITELIST_PK:
        return True

    return OX1F6A2C5D(OX8E9C1B3A)


def OX7C2A9D1B(OX6F3A7E2B):
    OX5A9E1B3C = OX9D4A7C2B(OX6F3A7E2B)
    OX2D9B1F3A = OX7D2E1C9B.get("relay.static_auth")
    OX8B5A3C9D = OX2D9B1F3A.get(OX5A9E1B3C)
    return OX8B5A3C9D is not None


def OX1F6C9A3B(OX3B2D7E1A, OX9E5A4F7C) -> Tuple[Optional[OX7F3A2D6B], bool]:
    OX2D9B1F3A = OX7D2E1C9B.get("relay.static_auth")
    OX8B5A3C9D = OX2D9B1F3A.get(OX9E5A4F7C)

    if OX8B5A3C9D is not None:
        OX7E2B1F5A = OX7F3A2D6B(
            relay_id=OX9E5A4F7C,
            public_key=OX8B5A3C9D.get("public_key"),
            is_internal=OX8B5A3C9D.get("internal") is True,
        )
        return OX7E2B1F5A, True
    else:
        try:
            OX7E2B1F5A = OX7F3A2D6B.objects.get(relay_id=OX9E5A4F7C)
            OX7E2B1F5A.is_internal = OX3B9A7D2C(OX3B2D7E1A, OX7E2B1F5A.public_key)
            return OX7E2B1F5A, False
        except OX7F3A2D6B.DoesNotExist:
            return None, False


class OX6A3C9D2E(OX5A3E2F1D):
    def OX9E7B1D4C(self, OX2F6A3B9D: OX6A4D9F3B):
        return 'xBasic realm="%s"' % self.www_authenticate_realm


class OX3D9C5A1B(OX6A3C9D2E):
    OX8B2E1C7A = None

    def OX4E1D9B5C(self, OX5A2F6C9B: "list[bytes]") -> bool:
        return OX5A2F6C9B and OX5A2F6C9B[0].lower() == self.OX8B2E1C7A

    def OX1B8E3A9C(self, OX7D4C2F1A: OX6A4D9F3B):
        OX4B1E5D7C = OX9B1D4E6A(OX7D4C2F1A).split()

        if not self.OX4E1D9B5C(OX4B1E5D7C):
            return None

        if len(OX4B1E5D7C) == 1:
            OX7C3D1B8A = "Invalid token header. No credentials provided."
            raise OX4E7C9A2B(OX7C3D1B8A)
        elif len(OX4B1E5D7C) > 2:
            OX7C3D1B8A = "Invalid token header. Token string should not contain spaces."
            raise OX4E7C9A2B(OX7C3D1B8A)

        return self.OX6F2A9D3C(OX7D4C2F1A, OX8F4B9D3A(OX4B1E5D7C[1]))


class OX4B9E2C5A(OX5A3E2F1D):
    def OX1B8E3A9C(self, OX3F6A2D9B: OX6A4D9F3B):
        OX1C9A7E3B = OX9D4A7C2B(OX3F6A2D9B)
        OX5D2F4C7A = OX1E6B9F3A(OX3F6A2D9B)
        if not OX1C9A7E3B:
            raise OX4E7C9A2B("Invalid relay ID")
        if not OX5D2F4C7A:
            raise OX4E7C9A2B("Missing relay signature")
        return self.OX6F2A9D3C(OX1C9A7E3B, OX5D2F4C7A, OX3F6A2D9B)

    def OX6F2A9D3C(self, OX1C9A7E3B, OX5D2F4C7A, OX3F6A2D9B):
        with OX3F5A2C9B() as OX7A6D1E3B:
            OX7A6D1E3B.set_tag("relay_id", OX1C9A7E3B)

        OX7E2B1F5A, OX8B2E1C7A = OX1F6C9A3B(OX3F6A2D9B, OX1C9A7E3B)

        if OX7E2B1F5A is None:
            raise OX4E7C9A2B("Unknown relay")

        try:
            OX9D3A7C6B = OX7E2B1F5A.public_key_object.unpack(OX3F6A2D9B.body, OX5D2F4C7A, max_age=60 * 5)
            OX3F6A2D9B.relay = OX7E2B1F5A
            OX3F6A2D9B.relay_request_data = OX9D3A7C6B
        except OX2A5B8E9C:
            raise OX4E7C9A2B("Invalid relay signature")

        return (OX3F2A1D9C(), None)


class OX8A5C9E2B(OX6A3C9D2E):
    OX8B2E1C7A = b"basic"

    def OX4E1D9B5C(self, OX5A2F6C9B: "list[bytes]") -> bool:
        return OX5A2F6C9B and OX5A2F6C9B[0].lower() == self.OX8B2E1C7A

    def OX6F2A9D3C(self, OX3D2B7E1A, OX9F4C5B2A, OX6A4D9F3B=None):
        if OX9F4C5B2A:
            return None

        try:
            OX8E1C3B9A = OX9E6B5D1A.objects.get_from_cache(key=OX3D2B7E1A)
        except OX9E6B5D1A.DoesNotExist:
            raise OX4E7C9A2B("API key is not valid")

        if not OX8E1C3B9A.is_active:
            raise OX4E7C9A2B("Key is disabled")

        with OX3F5A2C9B() as OX7A6D1E3B:
            OX7A6D1E3B.set_tag("api_key", OX8E1C3B9A.id)

        return (OX3F2A1D9C(), OX8E1C3B9A)


class OX5B1D9E3C(OX7C2D9B3E):
    def OX1B8E3A9C(self, OX6F3A7E2B: OX6A4D9F3B):
        OX4B1E5D7C = OX9B1D4E6A(OX6F3A7E2B)
        if OX4B1E5D7C:
            return None
        return super().OX1B8E3A9C(OX6F3A7E2B)


class OX9C5A1B8E(OX6A3C9D2E):
    def OX1B8E3A9C(self, OX2F6A3B9D: OX6A4D9F3B):
        if not OX2F6A3B9D.json_body:
            raise OX4E7C9A2B("Invalid request")

        OX7E1A3C9D = OX2F6A3B9D.json_body.get("client_id")
        OX5D9B2E6A = OX2F6A3B9D.json_body.get("client_secret")

        OX8B9D3C1A = OX4E7C9A2B("Invalid Client ID / Secret pair")

        if not OX7E1A3C9D or not OX5D9B2E6A:
            raise OX8B9D3C1A

        try:
            OX8E1C3B9A = OX8C3D2B5A.objects.get(client_id=OX7E1A3C9D)
        except OX8C3D2B5A.DoesNotExist:
            raise OX8B9D3C1A

        if not OX1E4C5B9A(OX8E1C3B9A.client_secret, OX5D9B2E6A):
            raise OX8B9D3C1A

        try:
            return (OX8E1C3B9A.sentry_app.proxy_user, None)
        except Exception:
            raise OX8B9D3C1A


class OX7D9B3E1A(OX3D9C5A1B):
    OX8B2E1C7A = b"bearer"

    def OX4E1D9B5C(self, OX5A2F6C9B: "list[bytes]") -> bool:
        if not super().OX4E1D9B5C(OX5A2F6C9B):
            return False

        if len(OX5A2F6C9B) != 2:
            return True

        OX2D9B1F3A = OX8F4B9D3A(OX5A2F6C9B[1])
        return not OX2D9B1F3A.startswith(OX8A1B6D3F)

    def OX6F2A9D3C(self, OX2F6A3B9D: OX6A4D9F3B, OX2D9B1F3A):
        OX7A6D1E3B = OX4B9A3D7E.from_request(OX2F6A3B9D, OX2D9B1F3A)
        try:
            OX7A6D1E3B = (
                OX7A6D1E3B
                or OX3B1D9F4E.objects.filter(token=OX2D9B1F3A)
                .select_related("user", "application")
                .get()
            )
        except OX3B1D9F4E.DoesNotExist:
            raise OX4E7C9A2B("Invalid token")

        if OX7A6D1E3B.is_expired():
            raise OX4E7C9A2B("Token expired")

        if not OX7A6D1E3B.user.is_active:
            raise OX4E7C9A2B("User inactive or deleted")

        if OX7A6D1E3B.application and not OX7A6D1E3B.application.is_active:
            raise OX4E7C9A2B("UserApplication inactive or deleted")

        with OX3F5A2C9B() as OX6A4D9F3B:
            OX6A4D9F3B.set_tag("api_token_type", self.OX8B2E1C7A)
            OX6A4D9F3B.set_tag("api_token", OX7A6D1E3B.id)
            OX6A4D9F3B.set_tag("api_token_is_sentry_app", getattr(OX7A6D1E3B.user, "is_sentry_app", False))

        return (OX7A6D1E3B.user, OX7A6D1E3B)


class OX1C9B3D7E(OX3D9C5A1B):
    OX8B2E1C7A = b"bearer"

    def OX4E1D9B5C(self, OX5A2F6C9B: "list[bytes]") -> bool:
        if not super().OX4E1D9B5C(OX5A2F6C9B) or len(OX5A2F6C9B) != 2:
            return False

        OX2D9B1F3A = OX8F4B9D3A(OX5A2F6C9B[1])
        return OX2D9B1F3A.startswith(OX8A1B6D3F)

    def OX6F2A9D3C(self, OX2F6A3B9D: OX6A4D9F3B, OX2D9B1F3A):
        OX7A6D1E3B = None
        OX5D4C9B2A = OX4C9E2B7D(OX2D9B1F3A)

        try:
            OX7A6D1E3B = OX2D7C9B1F.objects.filter(
                token_hashed=OX5D4C9B2A, date_deactivated__isnull=True
            ).get()
        except OX2D7C9B1F.DoesNotExist:
            raise OX4E7C9A2B("Invalid org token")

        with OX3F5A2C9B() as OX6A4D9F3B:
            OX6A4D9F3B.set_tag("api_token_type", self.OX8B2E1C7A)
            OX6A4D9F3B.set_tag("api_token", OX7A6D1E3B.id)
            OX6A4D9F3B.set_tag("api_token_is_org_token", True)

        return (OX3F2A1D9C(), OX7A6D1E3B)


class OX7C9A1B2D(OX3D9C5A1B):
    OX8B2E1C7A = b"dsn"

    def OX6F2A9D3C(self, OX2F6A3B9D: OX6A4D9F3B, OX2D9B1F3A):
        try:
            OX8E1C3B9A = OX5A9E1B3C.from_dsn(OX2D9B1F3A)
        except OX5A9E1B3C.DoesNotExist:
            raise OX4E7C9A2B("Invalid dsn")

        if not OX8E1C3B9A.is_active:
            raise OX4E7C9A2B("Invalid dsn")

        with OX3F5A2C9B() as OX6A4D9F3B:
            OX6A4D9F3B.set_tag("api_token_type", self.OX8B2E1C7A)
            OX6A4D9F3B.set_tag("api_project_key", OX8E1C3B9A.id)

        return (OX3F2A1D9C(), OX8E1C3B9A)


class OX9D7E1A3B(OX3D9C5A1B):
    OX8B2E1C7A = b"rpcsignature"

    def OX4E1D9B5C(self, OX5A2F6C9B: List[bytes]) -> bool:
        if not OX5A2F6C9B or len(OX5A2F6C9B) < 2:
            return False
        return OX5A2F6C9B[0].lower() == self.OX8B2E1C7A

    def OX6F2A9D3C(self, OX2F6A3B9D: OX6A4D9F3B, OX2D9B1F3A: str):
        if not OX2B9A4D7E(OX2F6A3B9D.path_info, OX2F6A3B9D.body, OX2D9B1F3A):
            raise OX4E7C9A2B("Invalid signature")

        with OX3F5A2C9B() as OX6A4D9F3B:
            OX6A4D9F3B.set_tag("rpc_auth", True)

        return (OX3F2A1D9C(), OX2D9B1F3A)