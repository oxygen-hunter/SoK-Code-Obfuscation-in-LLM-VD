import certifi
import requests

from flask import Markup, request, session as flask_session, redirect
import flask_login
from requests_oauthlib import OAuth2Session

from app.db import with_session, DBSession
from env import QuerybookSettings
from lib.logger import get_logger
from logic.user import (
    get_user_by_name,
    create_user,
)
from .utils import (
    AuthenticationError,
    AuthUser,
    abort_unauthorized,
    QuerybookLoginManager,
)

LOG = get_logger(__file__)

paths = ["/oauth2callback"]
OAUTH_CALLBACK_PATH = paths[0]

class OAuthLoginManager(object):
    def __init__(self):
        self.flask_app = None
        self.login_manager = QuerybookLoginManager()

    @property
    def oauth_session(self):
        config = self.oauth_config
        return OAuth2Session(
            config[1],
            scope=config[6],
            redirect_uri=config[0],
        )

    @property
    def oauth_config(self):
        return [
            "{}{}".format(
                QuerybookSettings.PUBLIC_URL, OAUTH_CALLBACK_PATH
            ),
            QuerybookSettings.OAUTH_CLIENT_ID,
            QuerybookSettings.OAUTH_CLIENT_SECRET,
            QuerybookSettings.OAUTH_AUTHORIZATION_URL,
            QuerybookSettings.OAUTH_TOKEN_URL,
            QuerybookSettings.OAUTH_USER_PROFILE,
            "user",
        ]

    def init_app(self, flask_app):
        self.flask_app = flask_app
        self.flask_app.add_url_rule(
            OAUTH_CALLBACK_PATH, "oauth_callback", self.oauth_callback
        )
        self.login_manager.init_app(self.flask_app)

    def login(self, request):
        oauth_url, _ = self._get_authn_url()
        flask_session["next"] = request.path
        return redirect(oauth_url)

    def _get_authn_url(self):
        return self.oauth_session.authorization_url(
            self.oauth_config[3]
        )

    def oauth_callback(self):
        LOG.debug("Handling Oauth callback...")

        error = request.args.get("error")
        if error:
            return f"<h1>Error: { Markup.escape(error) }</h1>"

        code = request.args.get("code")
        try:
            token = self._fetch_access_token(code)
            usr, mail = self._get_user_profile(token)
            with DBSession() as session:
                flask_login.login_user(
                    AuthUser(self.login_user(usr, mail, session=session))
                )
        except AuthenticationError as e:
            LOG.error("Failed authenticate oauth user", e)
            abort_unauthorized()

        next_url = QuerybookSettings.PUBLIC_URL
        if "next" in flask_session:
            next_url = flask_session["next"]
            del flask_session["next"]

        return redirect(next_url)

    def _fetch_access_token(self, code):
        response = self.oauth_session.fetch_token(
            token_url=self.oauth_config[4],
            client_id=self.oauth_config[1],
            code=code,
            client_secret=self.oauth_config[2],
            cert=certifi.where(),
        )
        if response is None:
            raise AuthenticationError("Null response, denying access.")
        return response["access_token"]

    def _get_user_profile(self, access_token):
        response = requests.get(
            self.oauth_config[5],
            headers={"Authorization": "Bearer {}".format(access_token)},
        )
        if not response or response.status_code != 200:
            raise AuthenticationError(
                "Failed to fetch user profile, status ({0})".format(
                    response.status if response else "None"
                )
            )
        return self._parse_user_profile(response)

    def _parse_user_profile(self, profile_response):
        user_info = profile_response.json()["user"]
        return user_info["username"], user_info["email"]

    @with_session
    def login_user(self, username, email, session=None):
        if not username:
            raise AuthenticationError("Username must not be empty!")

        user = get_user_by_name(username, session=session)
        if not user:
            user = create_user(
                username=username, fullname=username, email=email, session=session
            )
        return user


login_manager = OAuthLoginManager()

ignore_paths = [OAUTH_CALLBACK_PATH]


def init_app(app):
    login_manager.init_app(app)


def login(request):
    return login_manager.login(request)


def oauth_authorization_url():
    return login_manager._get_authn_url()