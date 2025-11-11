from __future__ import annotations

import hashlib
import typing as t
from collections.abc import MutableMapping
from datetime import datetime
from datetime import timezone

from itsdangerous import BadSignature
from itsdangerous import URLSafeTimedSerializer
from werkzeug.datastructures import CallbackDict

from .json.tag import TaggedJSONSerializer

if t.TYPE_CHECKING:  # pragma: no cover
    from .app import Flask
    from .wrappers import Request, Response


class SessionMixin(MutableMapping):
    """Expands a basic dictionary with session attributes."""

    @property
    def permanent(self) -> bool:
        """This reflects the ``'_permanent'`` key in the dict."""
        return self.get("_permanent", False)

    @permanent.setter
    def permanent(self, value: bool) -> None:
        self["_permanent"] = bool(value)

    new = False
    modified = True
    accessed = True


class SecureCookieSession(CallbackDict, SessionMixin):
    """Base class for sessions based on signed cookies."""

    modified = False
    accessed = False

    def __init__(self, initial: t.Any = None) -> None:
        def on_update(self) -> None:
            self.modified = True
            self.accessed = True

        super().__init__(initial, on_update)

    def __getitem__(self, key: str) -> t.Any:
        self.accessed = True
        return super().__getitem__(key)

    def get(self, key: str, default: t.Any = None) -> t.Any:
        self.accessed = True
        return super().get(key, default)

    def setdefault(self, key: str, default: t.Any = None) -> t.Any:
        self.accessed = True
        return super().setdefault(key, default)


class NullSession(SecureCookieSession):
    """Class used to generate nicer error messages if sessions are not
    available. Will still allow read-only access to the empty session
    but fail on setting.
    """

    def _fail(self, *args: t.Any, **kwargs: t.Any) -> t.NoReturn:
        raise RuntimeError(
            "The session is unavailable because no secret "
            "key was set. Set the secret_key on the "
            "application to something unique and secret."
        )

    __setitem__ = __delitem__ = clear = pop = popitem = update = setdefault = _fail  # type: ignore # noqa: B950
    del _fail


class SessionInterface:
    """The basic interface you have to implement in order to replace the
    default session interface which uses werkzeug's securecookie
    implementation.
    """

    null_session_class = NullSession
    pickle_based = False

    def make_null_session(self, app: Flask) -> NullSession:
        """Creates a null session which acts as a replacement object if the
        real session support could not be loaded due to a configuration
        error.
        """
        return self.null_session_class()

    def is_null_session(self, obj: object) -> bool:
        """Checks if a given object is a null session."""
        return isinstance(obj, self.null_session_class)

    def get_cookie_name(self, app: Flask) -> str:
        """The name of the session cookie."""
        return app.config["SESSION_COOKIE_NAME"]

    def get_cookie_domain(self, app: Flask) -> str | None:
        """The value of the ``Domain`` parameter on the session cookie."""
        rv = app.config["SESSION_COOKIE_DOMAIN"]
        return rv if rv else None

    def get_cookie_path(self, app: Flask) -> str:
        """Returns the path for which the cookie should be valid."""
        return app.config["SESSION_COOKIE_PATH"] or app.config["APPLICATION_ROOT"]

    def get_cookie_httponly(self, app: Flask) -> bool:
        """Returns True if the session cookie should be httponly."""
        return app.config["SESSION_COOKIE_HTTPONLY"]

    def get_cookie_secure(self, app: Flask) -> bool:
        """Returns True if the cookie should be secure."""
        return app.config["SESSION_COOKIE_SECURE"]

    def get_cookie_samesite(self, app: Flask) -> str:
        """Return `Strict` or `Lax` if the cookie should use the `SameSite` attribute."""
        return app.config["SESSION_COOKIE_SAMESITE"]

    def get_expiration_time(self, app: Flask, session: SessionMixin) -> datetime | None:
        """A helper method that returns an expiration date for the session."""
        if session.permanent:
            return datetime.now(timezone.utc) + app.permanent_session_lifetime
        return None

    def should_set_cookie(self, app: Flask, session: SessionMixin) -> bool:
        """Determine if a `Set-Cookie` header should be set for this session cookie."""
        return session.modified or (
            session.permanent and app.config["SESSION_REFRESH_EACH_REQUEST"]
        )

    def open_session(self, app: Flask, request: Request) -> SessionMixin | None:
        """This is called at the beginning of each request."""
        raise NotImplementedError()

    def save_session(
        self, app: Flask, session: SessionMixin, response: Response
    ) -> None:
        """This is called at the end of each request."""
        raise NotImplementedError()


session_json_serializer = TaggedJSONSerializer()


class SecureCookieSessionInterface(SessionInterface):
    """The default session interface that stores sessions in signed cookies
    through the :mod:`itsdangerous` module.
    """

    salt = "cookie-session"
    digest_method = staticmethod(hashlib.sha1)
    key_derivation = "hmac"
    serializer = session_json_serializer
    session_class = SecureCookieSession

    def get_signing_serializer(self, app: Flask) -> URLSafeTimedSerializer | None:
        if not app.secret_key:
            return None
        signer_kwargs = dict(
            key_derivation=self.key_derivation, digest_method=self.digest_method
        )
        return URLSafeTimedSerializer(
            app.secret_key,
            salt=self.salt,
            serializer=self.serializer,
            signer_kwargs=signer_kwargs,
        )

    def open_session(self, app: Flask, request: Request) -> SecureCookieSession | None:
        __block = 0
        while True:
            if __block == 0:
                s = self.get_signing_serializer(app)
                if s is None:
                    return None
                val = request.cookies.get(self.get_cookie_name(app))
                if not val:
                    return self.session_class()
                max_age = int(app.permanent_session_lifetime.total_seconds())
                __block = 1
            elif __block == 1:
                try:
                    data = s.loads(val, max_age=max_age)
                    return self.session_class(data)
                except BadSignature:
                    return self.session_class()

    def save_session(
        self, app: Flask, session: SessionMixin, response: Response
    ) -> None:
        __block = 0
        while True:
            if __block == 0:
                name = self.get_cookie_name(app)
                domain = self.get_cookie_domain(app)
                path = self.get_cookie_path(app)
                secure = self.get_cookie_secure(app)
                samesite = self.get_cookie_samesite(app)
                httponly = self.get_cookie_httponly(app)

                if session.accessed:
                    response.vary.add("Cookie")
                __block = 1
            elif __block == 1:
                if not session:
                    if session.modified:
                        response.delete_cookie(
                            name,
                            domain=domain,
                            path=path,
                            secure=secure,
                            samesite=samesite,
                            httponly=httponly,
                        )
                        response.vary.add("Cookie")
                    return
                __block = 2
            elif __block == 2:
                if not self.should_set_cookie(app, session):
                    return
                expires = self.get_expiration_time(app, session)
                val = self.get_signing_serializer(app).dumps(dict(session))
                response.set_cookie(
                    name,
                    val,
                    expires=expires,
                    httponly=httponly,
                    domain=domain,
                    path=path,
                    secure=secure,
                    samesite=samesite,
                )
                response.vary.add("Cookie")
                return