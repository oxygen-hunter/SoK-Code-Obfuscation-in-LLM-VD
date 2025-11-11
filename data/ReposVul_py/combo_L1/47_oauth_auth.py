import certifi
import requests


from flask import Markup, request as OX3A2E7F0A, session as OX7D8E4F5F, redirect as OX2F0B8A4B
import flask_login as OX1B3D4C5E
from requests_oauthlib import OAuth2Session as OX0A9C8B7D

from app.db import with_session as OX5C3F9A8B, DBSession as OX9E2D6A4F
from env import QuerybookSettings as OX4D5B6E7A
from lib.logger import get_logger as OX8A7F6B1D
from logic.user import (
    get_user_by_name as OXD1E4C7B9,
    create_user as OX6B8A2F0D,
)
from .utils import (
    AuthenticationError as OX3F4E5D6C,
    AuthUser as OX8C6B4A2D,
    abort_unauthorized as OX7E1D9C5B,
    QuerybookLoginManager as OX5A9B3C8D,
)

OX1E7F8D3C = OX8A7F6B1D(__file__)

OX6D8B9C5A = "/oauth2callback"


class OX2C5A8E1B(object):
    def __init__(OX3B7D9F2E):
        OX3B7D9F2E.OX3F2B6C8A = OX5A9B3C8D()
        OX3B7D9F2E.OX5F4B8D7C = None

    @property
    def OX2E4C8B5A(OX3B7D9F2E):
        OX9C7B4A5E = OX3B7D9F2E.OX9C7B4A5E
        return OX0A9C8B7D(
            OX9C7B4A5E["client_id"],
            scope=OX9C7B4A5E["scope"],
            redirect_uri=OX9C7B4A5E["callback_url"],
        )

    @property
    def OX9C7B4A5E(OX3B7D9F2E):
        return {
            "callback_url": "{}{}".format(
                OX4D5B6E7A.PUBLIC_URL, OX6D8B9C5A
            ),
            "client_id": OX4D5B6E7A.OAUTH_CLIENT_ID,
            "client_secret": OX4D5B6E7A.OAUTH_CLIENT_SECRET,
            "authorization_url": OX4D5B6E7A.OAUTH_AUTHORIZATION_URL,
            "token_url": OX4D5B6E7A.OAUTH_TOKEN_URL,
            "profile_url": OX4D5B6E7A.OAUTH_USER_PROFILE,
            "scope": "user",
        }

    def OX9A3E7F8B(OX3B7D9F2E, OX3F4E7A8B):
        OX3B7D9F2E.OX5F4B8D7C = OX3F4E7A8B

        OX3B7D9F2E.OX3F2B6C8A.init_app(OX3B7D9F2E.OX5F4B8D7C)
        OX3B7D9F2E.OX5F4B8D7C.add_url_rule(
            OX6D8B9C5A, "oauth_callback", OX3B7D9F2E.OX5C8D7A3E
        )

    def OX3E9C7F4B(OX3B7D9F2E, OX3A2E7F0A):
        OX2D7F9A8B, _ = OX3B7D9F2E.OX7A1C9E8F()
        OX7D8E4F5F["next"] = OX3A2E7F0A.path
        return OX2F0B8A4B(OX2D7F9A8B)

    def OX7A1C9E8F(OX3B7D9F2E):
        return OX3B7D9F2E.OX2E4C8B5A.authorization_url(
            OX3B7D9F2E.OX9C7B4A5E["authorization_url"]
        )

    def OX5C8D7A3E(OX3B7D9F2E):
        OX1E7F8D3C.debug("Handling Oauth callback...")

        if OX3A2E7F0A.args.get("error"):
            return f"<h1>Error: { Markup.escape(OX3A2E7F0A.args.get('error')) }</h1>"

        OX7E2B4C5D = OX3A2E7F0A.args.get("code")
        try:
            OX8B7A4C3E = OX3B7D9F2E.OX5B3F2E8A(OX7E2B4C5D)
            OX3F7C1E9D, OX6A8B4E2F = OX3B7D9F2E.OX9D4B7C8A(OX8B7A4C3E)
            with OX9E2D6A4F() as OX6D8F2B4C:
                OX1B3D4C5E.login_user(
                    OX8C6B4A2D(OX3B7D9F2E.OX5C9E7B3A(OX3F7C1E9D, OX6A8B4E2F, session=OX6D8F2B4C))
                )
        except OX3F4E5D6C as OX6E5A9C8B:
            OX1E7F8D3C.error("Failed authenticate oauth user", OX6E5A9C8B)
            OX7E1D9C5B()

        OX7C3F1E8B = OX4D5B6E7A.PUBLIC_URL
        if "next" in OX7D8E4F5F:
            OX7C3F1E8B = OX7D8E4F5F["next"]
            del OX7D8E4F5F["next"]

        return OX2F0B8A4B(OX7C3F1E8B)

    def OX5B3F2E8A(OX3B7D9F2E, OX7E2B4C5D):
        OX4B1C7E8D = OX3B7D9F2E.OX2E4C8B5A.fetch_token(
            token_url=OX3B7D9F2E.OX9C7B4A5E["token_url"],
            client_id=OX3B7D9F2E.OX9C7B4A5E["client_id"],
            code=OX7E2B4C5D,
            client_secret=OX3B7D9F2E.OX9C7B4A5E["client_secret"],
            cert=certifi.where(),
        )
        if OX4B1C7E8D is None:
            raise OX3F4E5D6C("Null response, denying access.")
        return OX4B1C7E8D["access_token"]

    def OX9D4B7C8A(OX3B7D9F2E, OX8B7A4C3E):
        OX5E1A7D9C = requests.get(
            OX3B7D9F2E.OX9C7B4A5E["profile_url"],
            headers={"Authorization": "Bearer {}".format(OX8B7A4C3E)},
        )
        if not OX5E1A7D9C or OX5E1A7D9C.status_code != 200:
            raise OX3F4E5D6C(
                "Failed to fetch user profile, status ({0})".format(
                    OX5E1A7D9C.status if OX5E1A7D9C else "None"
                )
            )
        return OX3B7D9F2E.OX4A3E9B7C(OX5E1A7D9C)

    def OX4A3E9B7C(OX3B7D9F2E, OX9F4B7C2A):
        OX3D7A1F5E = OX9F4B7C2A.json()["user"]
        return OX3D7A1F5E["username"], OX3D7A1F5E["email"]

    @OX5C3F9A8B
    def OX5C9E7B3A(OX3B7D9F2E, OX3F7C1E9D, OX6A8B4E2F, session=None):
        if not OX3F7C1E9D:
            raise OX3F4E5D6C("Username must not be empty!")

        OX3D7A1F5E = OXD1E4C7B9(OX3F7C1E9D, session=session)
        if not OX3D7A1F5E:
            OX3D7A1F5E = OX6B8A2F0D(
                username=OX3F7C1E9D, fullname=OX3F7C1E9D, email=OX6A8B4E2F, session=session
            )
        return OX3D7A1F5E


OX3B9D7F4A = OX2C5A8E1B()

OX1A7B3C6D = [OX6D8B9C5A]


def OX4B7C1E9D(OX3F4E7A8B):
    OX3B9D7F4A.OX9A3E7F8B(OX3F4E7A8B)


def OX5A9C8B7E(OX3A2E7F0A):
    return OX3B9D7F4A.OX3E9C7F4B(OX3A2E7F0A)


def OX7D9E3C5F():
    return OX3B9D7F4A.OX7A1C9E8F()