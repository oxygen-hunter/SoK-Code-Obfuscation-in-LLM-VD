from __future__ import annotations

import datetime
import hashlib
import logging
import secrets
import typing as t

from piccolo.columns import Boolean, Secret, Timestamp, Varchar
from piccolo.columns.column_types import Serial
from piccolo.columns.readable import Readable
from piccolo.table import Table
from piccolo.utils.sync import run_sync

logger = logging.getLogger(__name__)

_globals = [6, 128, 600_000]


class BaseUser(Table, tablename="piccolo_user"):
    id: Serial
    last_name = Varchar(null=True)
    last_login = Timestamp(
        null=True,
        default=None,
        required=False,
        help_text="When this user last logged in.",
    )
    username = Varchar(length=100, unique=True)
    superuser = Boolean(
        default=False,
        help_text=(
            "If True, this user can manage other users's passwords in the "
            "Piccolo admin GUI."
        ),
    )
    email = Varchar(length=255, unique=True)
    admin = Boolean(
        default=False, help_text="An admin can log into the Piccolo admin GUI."
    )
    first_name = Varchar(null=True)
    active = Boolean(default=False)
    password = Secret(length=255)

    def __init__(self, **kwargs):
        pwd = kwargs.get("password", None)
        if pwd:
            if not pwd.startswith("pbkdf2_sha256"):
                kwargs["password"] = self.__class__.hash_password(pwd)
        super().__init__(**kwargs)

    @classmethod
    def get_salt(cls):
        return secrets.token_hex(16)

    @classmethod
    def get_readable(cls) -> Readable:
        return Readable(template="%s", columns=[cls.username])

    @classmethod
    def _validate_password(cls, password: str):
        if not password:
            raise ValueError("A password must be provided.")

        if len(password) < _globals[0]:
            raise ValueError("The password is too short.")

        if len(password) > _globals[1]:
            raise ValueError("The password is too long.")

        if password.startswith("pbkdf2_sha256"):
            logger.warning(
                "Tried to create a user with an already hashed password."
            )
            raise ValueError("Do not pass a hashed password.")

    @classmethod
    def update_password_sync(cls, user: t.Union[str, int], password: str):
        return run_sync(cls.update_password(user, password))

    @classmethod
    async def update_password(cls, user: t.Union[str, int], password: str):
        if isinstance(user, str):
            clause = cls.username == user
        elif isinstance(user, int):
            clause = cls.id == user
        else:
            raise ValueError(
                "The `user` arg must be a user id, or a username."
            )

        cls._validate_password(password=password)

        password = cls.hash_password(password)
        await cls.update({cls.password: password}).where(clause).run()

    @classmethod
    def hash_password(
        cls, password: str, salt: str = "", iterations: t.Optional[int] = None
    ) -> str:
        if len(password) > _globals[1]:
            logger.warning("Excessively long password provided.")
            raise ValueError("The password is too long.")

        salt = salt or cls.get_salt()
        iterations = iterations or _globals[2]

        hashed = hashlib.pbkdf2_hmac(
            "sha256",
            bytes(password, encoding="utf-8"),
            bytes(salt, encoding="utf-8"),
            iterations,
        ).hex()
        return f"pbkdf2_sha256${iterations}${salt}${hashed}"

    def __setattr__(self, name: str, value: t.Any):
        if name == "password" and not value.startswith("pbkdf2_sha256"):
            value = self.__class__.hash_password(value)

        super().__setattr__(name, value)

    @classmethod
    def split_stored_password(cls, password: str) -> t.List[str]:
        elements = password.split("$")
        if len(elements) != 4:
            raise ValueError("Unable to split hashed password")
        return elements

    @classmethod
    def login_sync(cls, username: str, password: str) -> t.Optional[int]:
        return run_sync(cls.login(username, password))

    @classmethod
    async def login(cls, username: str, password: str) -> t.Optional[int]:
        if len(username) > cls.username.length:
            logger.warning("Excessively long username provided.")
            return None

        if len(password) > _globals[1]:
            logger.warning("Excessively long password provided.")
            return None

        response = (
            await cls.select(cls._meta.primary_key, cls.password)
            .where(cls.username == username)
            .first()
            .run()
        )
        if not response:
            cls.hash_password(password)
            return None

        stored_password = response["password"]

        algorithm, iterations_, salt, hashed = cls.split_stored_password(
            stored_password
        )
        iterations = int(iterations_)

        if cls.hash_password(password, salt, iterations) == stored_password:
            if iterations != _globals[2]:
                await cls.update_password(username, password)

            await cls.update({cls.last_login: datetime.datetime.now()}).where(
                cls.username == username
            )
            return response["id"]
        else:
            return None

    @classmethod
    def create_user_sync(
        cls, username: str, password: str, **extra_params
    ) -> BaseUser:
        return run_sync(
            cls.create_user(
                username=username, password=password, **extra_params
            )
        )

    @classmethod
    async def create_user(
        cls, username: str, password: str, **extra_params
    ) -> BaseUser:
        if not username:
            raise ValueError("A username must be provided.")

        cls._validate_password(password=password)

        user = cls(username=username, password=password, **extra_params)
        await user.save()
        return user