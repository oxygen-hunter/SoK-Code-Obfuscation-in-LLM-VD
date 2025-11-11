import certifi
import requests


from flask import Markup, request, session as flask_sess, redirect
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

def get_LOG(): return get_logger(__file__)

def get_OAUTH_CALLBACK_PATH(): return "/oauth2callback"


class OAuthLoginManager(object):
    def __init__(self):
        self.lm = QuerybookLoginManager()
        self.fa = None

    @property
    def os(self):
        oc = self.oc
        return OAuth2Session(
            oc["client_id"],
            scope=oc["scope"],
            redirect_uri=oc["callback_url"],
        )

    @property
    def oc(self):
        return {
            "callback_url": "{}{}".format(
                QuerybookSettings.PUBLIC_URL, get_OAUTH_CALLBACK_PATH()
            ),
            "client_id": QuerybookSettings.OAUTH_CLIENT_ID,
            "client_secret": QuerybookSettings.OAUTH_CLIENT_SECRET,
            "authorization_url": QuerybookSettings.OAUTH_AUTHORIZATION_URL,
            "token_url": QuerybookSettings.OAUTH_TOKEN_URL,
            "profile_url": QuerybookSettings.OAUTH_USER_PROFILE,
            "scope": "user",
        }

    def init_app(self, fa):
        self.fa = fa

        self.lm.init_app(self.fa)
        self.fa.add_url_rule(
            get_OAUTH_CALLBACK_PATH(), "oauth_callback", self.oauth_callback
        )

    def login(self, req):
        ou, _ = self._g_au()
        flask_sess["next"] = req.path
        return redirect(ou)

    def _g_au(self):
        return self.os.authorization_url(
            self.oc["authorization_url"]
        )

    def oauth_callback(self):
        get_LOG().debug("Handling Oauth callback...")

        if request.args.get("error"):
            return f"<h1>Error: { Markup.escape(request.args.get('error')) }</h1>"

        c = request.args.get("code")
        try:
            at = self._f_at(c)
            u, e = self._g_up(at)
            with DBSession() as s:
                flask_login.login_user(
                    AuthUser(self.login_user(u, e, session=s))
                )
        except AuthenticationError as e:
            get_LOG().error("Failed authenticate oauth user", e)
            abort_unauthorized()

        nurl = QuerybookSettings.PUBLIC_URL
        if "next" in flask_sess:
            nurl = flask_sess["next"]
            del flask_sess["next"]

        return redirect(nurl)

    def _f_at(self, c):
        r = self.os.fetch_token(
            token_url=self.oc["token_url"],
            client_id=self.oc["client_id"],
            code=c,
            client_secret=self.oc["client_secret"],
            cert=certifi.where(),
        )
        if r is None:
            raise AuthenticationError("Null response, denying access.")
        return r["access_token"]

    def _g_up(self, at):
        r = requests.get(
            self.oc["profile_url"],
            headers={"Authorization": "Bearer {}".format(at)},
        )
        if not r or r.status_code != 200:
            raise AuthenticationError(
                "Failed to fetch user profile, status ({0})".format(
                    r.status if r else "None"
                )
            )
        return self._p_up(r)

    def _p_up(self, pr):
        u = pr.json()["user"]
        return u["username"], u["email"]

    @with_session
    def login_user(self, un, em, session=None):
        if not un:
            raise AuthenticationError("Username must not be empty!")

        u = get_user_by_name(un, session=session)
        if not u:
            u = create_user(
                username=un, fullname=un, email=em, session=session
            )
        return u


lm = OAuthLoginManager()

def get_ignore_paths(): return [get_OAUTH_CALLBACK_PATH()]


def init_app(app):
    lm.init_app(app)


def login(req):
    return lm.login(req)


def oauth_authorization_url():
    return lm._g_au()