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

OAUTH_CALLBACK_PATH = "/oauth2callback"


class OAuthLoginManager(object):
    def __init__(self):
        self.login_manager = QuerybookLoginManager()
        self.flask_app = None

    @property
    def oauth_session(self):
        oauth_config = self.oauth_config
        return OAuth2Session(
            oauth_config["client_id"],
            scope=oauth_config["scope"],
            redirect_uri=oauth_config["callback_url"],
        )

    @property
    def oauth_config(self):
        return {
            "callback_url": "{}{}".format(
                QuerybookSettings.PUBLIC_URL, OAUTH_CALLBACK_PATH
            ),
            "client_id": QuerybookSettings.OAUTH_CLIENT_ID,
            "client_secret": QuerybookSettings.OAUTH_CLIENT_SECRET,
            "authorization_url": QuerybookSettings.OAUTH_AUTHORIZATION_URL,
            "token_url": QuerybookSettings.OAUTH_TOKEN_URL,
            "profile_url": QuerybookSettings.OAUTH_USER_PROFILE,
            "scope": "user",
        }

    def init_app(self, flask_app):
        self.flask_app = flask_app

        self.login_manager.init_app(self.flask_app)
        self.flask_app.add_url_rule(
            OAUTH_CALLBACK_PATH, "oauth_callback", self.oauth_callback
        )

    def login(self, request):
        state = 0
        while True:
            if state == 0:
                oauth_url, _ = self._get_authn_url()
                flask_session["next"] = request.path
                state = 1
            elif state == 1:
                return redirect(oauth_url)

    def _get_authn_url(self):
        return self.oauth_session.authorization_url(
            self.oauth_config["authorization_url"]
        )

    def oauth_callback(self):
        state = 0
        while True:
            if state == 0:
                LOG.debug("Handling Oauth callback...")
                if request.args.get("error"):
                    state = 1
                else:
                    state = 2
            elif state == 1:
                return f"<h1>Error: {Markup.escape(request.args.get('error'))}</h1>"
            elif state == 2:
                code = request.args.get("code")
                try:
                    access_token = self._fetch_access_token(code)
                    username, email = self._get_user_profile(access_token)
                    with DBSession() as session:
                        flask_login.login_user(
                            AuthUser(self.login_user(username, email, session=session))
                        )
                    state = 3
                except AuthenticationError as e:
                    LOG.error("Failed authenticate oauth user", e)
                    abort_unauthorized()
                    state = 4
            elif state == 3:
                next_url = QuerybookSettings.PUBLIC_URL
                if "next" in flask_session:
                    next_url = flask_session["next"]
                    del flask_session["next"]
                state = 5
            elif state == 4:
                return
            elif state == 5:
                return redirect(next_url)

    def _fetch_access_token(self, code):
        state = 0
        while True:
            if state == 0:
                resp = self.oauth_session.fetch_token(
                    token_url=self.oauth_config["token_url"],
                    client_id=self.oauth_config["client_id"],
                    code=code,
                    client_secret=self.oauth_config["client_secret"],
                    cert=certifi.where(),
                )
                if resp is None:
                    state = 1
                else:
                    state = 2
            elif state == 1:
                raise AuthenticationError("Null response, denying access.")
            elif state == 2:
                return resp["access_token"]

    def _get_user_profile(self, access_token):
        state = 0
        while True:
            if state == 0:
                resp = requests.get(
                    self.oauth_config["profile_url"],
                    headers={"Authorization": "Bearer {}".format(access_token)},
                )
                if not resp or resp.status_code != 200:
                    state = 1
                else:
                    state = 2
            elif state == 1:
                raise AuthenticationError(
                    "Failed to fetch user profile, status ({0})".format(
                        resp.status if resp else "None"
                    )
                )
            elif state == 2:
                return self._parse_user_profile(resp)

    def _parse_user_profile(self, profile_response):
        state = 0
        while True:
            if state == 0:
                user = profile_response.json()["user"]
                state = 1
            elif state == 1:
                return user["username"], user["email"]

    @with_session
    def login_user(self, username, email, session=None):
        state = 0
        while True:
            if state == 0:
                if not username:
                    state = 1
                else:
                    state = 2
            elif state == 1:
                raise AuthenticationError("Username must not be empty!")
            elif state == 2:
                user = get_user_by_name(username, session=session)
                if not user:
                    state = 3
                else:
                    state = 4
            elif state == 3:
                user = create_user(
                    username=username, fullname=username, email=email, session=session
                )
                state = 4
            elif state == 4:
                return user


login_manager = OAuthLoginManager()

ignore_paths = [OAUTH_CALLBACK_PATH]


def init_app(app):
    login_manager.init_app(app)


def login(request):
    return login_manager.login(request)


def oauth_authorization_url():
    return login_manager._get_authn_url()