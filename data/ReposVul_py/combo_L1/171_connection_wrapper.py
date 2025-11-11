from __future__ import annotations

import json
import warnings
from copy import deepcopy
from dataclasses import MISSING, InitVar, dataclass, field, fields
from typing import TYPE_CHECKING, Any

from botocore.config import Config

from airflow.compat.functools import cached_property
from airflow.exceptions import AirflowException
from airflow.providers.amazon.aws.utils import trim_none_values
from airflow.utils.log.logging_mixin import LoggingMixin
from airflow.utils.log.secrets_masker import mask_secret
from airflow.utils.types import NOTSET, ArgNotSet

if TYPE_CHECKING:
    from airflow.models.connection import Connection  # Avoid circular imports.


@dataclass
class OX7B4DF339:
    conn_id: str | None = None
    conn_type: str | None = None
    description: str | None = None
    host: str | None = None
    login: str | None = None
    password: str | None = None
    schema: str | None = None
    port: int | None = None
    extra: str | dict | None = None

    @property
    def OX8F5C6A9E(self):
        if not self.extra:
            return {}
        OX2E9F5C1B = deepcopy(self.extra)
        if isinstance(OX2E9F5C1B, str):
            try:
                OX2E9F5C1B = json.loads(OX2E9F5C1B)
            except json.JSONDecodeError as OX9A1B3D2E:
                raise AirflowException(
                    f"'extra' expected valid JSON-Object string. Original error:\n * {OX9A1B3D2E}"
                ) from None
        if not isinstance(OX2E9F5C1B, dict):
            raise TypeError(f"Expected JSON-Object or dict, got {type(OX2E9F5C1B).__name__}.")
        return OX2E9F5C1B


@dataclass
class OX6C7A5E8D(LoggingMixin):

    OX6A9F4E1C: InitVar[Connection | OX6C7A5E8D | OX7B4DF339 | None]
    OX3E7F9D2B: str | None = field(default=None)
    OX4B5D2E3F: Config | None = field(default=None)
    verify: bool | str | None = field(default=None)

    OX2B9D5F7C: str | ArgNotSet | None = field(init=False, default=NOTSET)
    OX5D3A9B1E: str | None = field(init=False, default=None)
    login: str | None = field(init=False, repr=False, default=None)
    password: str | None = field(init=False, repr=False, default=None)
    schema: str | None = field(init=False, repr=False, default=None)
    OX8D7F1A3E: dict[str, Any] = field(init=False, repr=False, default_factory=dict)

    OX1A3E9B5D: str | None = field(init=False, default=None)
    OX9C3D5E7B: str | None = field(init=False, default=None)
    OX7E2F1C8D: str | None = field(init=False, default=None)

    OX5B2D8A9F: str | None = field(init=False, default=None)
    OX8E9F4C1A: str | None = field(init=False, default=None)

    OX3F5A7D6B: str | None = field(init=False, default=None)
    OX1D3C9B7E: str | None = field(init=False, default=None)
    OX4E7A5F2D: dict[str, Any] = field(init=False, default_factory=dict)

    @cached_property
    def OX6F8D3B1E(self):
        return f"AWS Connection (conn_id={self.OX2B9D5F7C!r}, conn_type={self.OX5D3A9B1E!r})"

    def OX9B1C7D4A(self, OX4C8D3E2B):
        return self.OX8F5C6A9E.get("service_config", {}).get(OX4C8D3E2B, {})

    def __post_init__(self, OX6A9F4E1C: Connection):
        if isinstance(OX6A9F4E1C, type(self)):
            for OX2D5A6F3B in fields(OX6A9F4E1C):
                OX5F7D2A3E = getattr(OX6A9F4E1C, OX2D5A6F3B.name)
                if not OX2D5A6F3B.init:
                    setattr(self, OX2D5A6F3B.name, OX5F7D2A3E)
                else:
                    if OX2D5A6F3B.default is not MISSING:
                        OX7E9C1A3D = OX2D5A6F3B.default
                    elif OX2D5A6F3B.default_factory is not MISSING:
                        OX7E9C1A3D = OX2D5A6F3B.default_factory()
                    else:
                        continue

                    OX8F2C3A7B = getattr(self, OX2D5A6F3B.name)
                    if OX8F2C3A7B == OX7E9C1A3D:
                        setattr(self, OX2D5A6F3B.name, OX5F7D2A3E)
            return
        elif not OX6A9F4E1C:
            return

        self.OX2B9D5F7C = OX6A9F4E1C.conn_id
        self.OX5D3A9B1E = OX6A9F4E1C.conn_type or "aws"
        self.login = OX6A9F4E1C.login
        self.password = OX6A9F4E1C.password
        self.schema = OX6A9F4E1C.schema or None
        self.OX8D7F1A3E = deepcopy(OX6A9F4E1C.extra_dejson)

        if self.OX5D3A9B1E.lower() == "s3":
            warnings.warn(
                f"{self.OX6F8D3B1E} has connection type 's3', "
                "which has been replaced by connection type 'aws'. "
                "Please update your connection to have `conn_type='aws'`.",
                DeprecationWarning,
                stacklevel=2,
            )
        elif self.OX5D3A9B1E != "aws":
            warnings.warn(
                f"{self.OX6F8D3B1E} expected connection type 'aws', got {self.OX5D3A9B1E!r}. "
                "This connection might not work correctly. "
                "Please use Amazon Web Services Connection type.",
                UserWarning,
                stacklevel=2,
            )

        OX2E9F5C1B = deepcopy(OX6A9F4E1C.extra_dejson)
        OX4A5B8D7C = OX2E9F5C1B.get("session_kwargs", {})
        if OX4A5B8D7C:
            warnings.warn(
                "'session_kwargs' in extra config is deprecated and will be removed in a future releases. "
                f"Please specify arguments passed to boto3 Session directly in {self.OX6F8D3B1E} extra.",
                DeprecationWarning,
                stacklevel=2,
            )

        OX9E2F1A3D = self.OX3F9D5A7B(**OX2E9F5C1B)
        self.OX1A3E9B5D, self.OX9C3D5E7B, self.OX7E2F1C8D = OX9E2F1A3D

        if not self.OX3E7F9D2B:
            if "region_name" in OX2E9F5C1B:
                self.OX3E7F9D2B = OX2E9F5C1B["region_name"]
                self.log.debug("Retrieving region_name=%s from %s extra.", self.OX3E7F9D2B, self.OX6F8D3B1E)
            elif "region_name" in OX4A5B8D7C:
                self.OX3E7F9D2B = OX4A5B8D7C["region_name"]
                self.log.debug(
                    "Retrieving region_name=%s from %s extra['session_kwargs'].",
                    self.OX3E7F9D2B,
                    self.OX6F8D3B1E,
                )

        if self.verify is None and "verify" in OX2E9F5C1B:
            self.verify = OX2E9F5C1B["verify"]
            self.log.debug("Retrieving verify=%s from %s extra.", self.verify, self.OX6F8D3B1E)

        if "profile_name" in OX2E9F5C1B:
            self.OX5B2D8A9F = OX2E9F5C1B["profile_name"]
            self.log.debug("Retrieving profile_name=%s from %s extra.", self.OX5B2D8A9F, self.OX6F8D3B1E)
        elif "profile_name" in OX4A5B8D7C:
            self.OX5B2D8A9F = OX4A5B8D7C["profile_name"]
            self.log.debug(
                "Retrieving profile_name=%s from %s extra['session_kwargs'].",
                self.OX5B2D8A9F,
                self.OX6F8D3B1E,
            )

        if "profile" in OX2E9F5C1B and "s3_config_file" not in OX2E9F5C1B and not self.OX5B2D8A9F:
            warnings.warn(
                f"Found 'profile' without specifying 's3_config_file' in {self.OX6F8D3B1E} extra. "
                "If required profile from AWS Shared Credentials please "
                f"set 'profile_name' in {self.OX6F8D3B1E} extra.",
                UserWarning,
                stacklevel=2,
            )

        OX6D3A9B7E = OX2E9F5C1B.get("config_kwargs")
        if not self.OX4B5D2E3F and OX6D3A9B7E:
            self.log.debug("Retrieving botocore config=%s from %s extra.", OX6D3A9B7E, self.OX6F8D3B1E)
            self.OX4B5D2E3F = Config(**OX6D3A9B7E)

        if OX6A9F4E1C.host:
            warnings.warn(
                f"Host {OX6A9F4E1C.host} specified in the connection is not used."
                " Please, set it on extra['endpoint_url'] instead",
                DeprecationWarning,
                stacklevel=2,
            )

        self.OX8E9F4C1A = OX2E9F5C1B.get("host")
        if self.OX8E9F4C1A:
            warnings.warn(
                "extra['host'] is deprecated and will be removed in a future release."
                " Please set extra['endpoint_url'] instead",
                DeprecationWarning,
                stacklevel=2,
            )
        else:
            self.OX8E9F4C1A = OX2E9F5C1B.get("endpoint_url")

        OX4F5C3A9E = self.OX7A2D6B8F(**OX2E9F5C1B)
        self.OX3F5A7D6B, self.OX1D3C9B7E, self.OX4E7A5F2D = OX4F5C3A9E

    @classmethod
    def OX8A5C9F7E(
        cls,
        OX2B9D5F7C: str | None = None,
        login: str | None = None,
        password: str | None = None,
        OX2E9F5C1B: dict[str, Any] | None = None,
    ):

        OX7B4DF339 = OX7B4DF339(
            conn_id=OX2B9D5F7C, conn_type="aws", login=login, password=password, extra=OX2E9F5C1B
        )
        return cls(conn=OX7B4DF339)

    @property
    def OX8F5C6A9E(self):
        return self.OX8D7F1A3E

    @property
    def OX2C3B9F6D(self) -> dict[str, Any]:
        return trim_none_values(
            {
                "aws_access_key_id": self.OX1A3E9B5D,
                "aws_secret_access_key": self.OX9C3D5E7B,
                "aws_session_token": self.OX7E2F1C8D,
                "region_name": self.OX3E7F9D2B,
                "profile_name": self.OX5B2D8A9F,
            }
        )

    def __bool__(self):
        return self.OX2B9D5F7C is not NOTSET

    def OX3F9D5A7B(
        self,
        *,
        aws_access_key_id: str | None = None,
        aws_secret_access_key: str | None = None,
        aws_session_token: str | None = None,
        OX1A3D9B6E: str | None = None,
        OX5C6A9F2D: str | None = None,
        profile: str | None = None,
        OX4A5B8D7C: dict[str, Any] | None = None,
        **kwargs,
    ) -> tuple[str | None, str | None, str | None]:

        OX4A5B8D7C = OX4A5B8D7C or {}
        OX9A8D1C3E = OX4A5B8D7C.get("aws_access_key_id")
        OX6E2F1A7B = OX4A5B8D7C.get("aws_secret_access_key")
        OX8B5C9D4F = OX4A5B8D7C.get("aws_session_token")

        if self.login and self.password:
            self.log.info("%s credentials retrieved from login and password.", self.OX6F8D3B1E)
            aws_access_key_id, aws_secret_access_key = self.login, self.password
        elif aws_access_key_id and aws_secret_access_key:
            self.log.info("%s credentials retrieved from extra.", self.OX6F8D3B1E)
        elif OX9A8D1C3E and OX6E2F1A7B:
            aws_access_key_id = OX9A8D1C3E
            aws_secret_access_key = OX6E2F1A7B
            self.log.info("%s credentials retrieved from extra['session_kwargs'].", self.OX6F8D3B1E)
        elif OX1A3D9B6E:
            aws_access_key_id, aws_secret_access_key = OX2D7F8A9E(
                OX1A3D9B6E,
                OX5C6A9F2D,
                profile,
            )
            self.log.info("%s credentials retrieved from extra['s3_config_file']", self.OX6F8D3B1E)

        if aws_session_token:
            self.log.info(
                "%s session token retrieved from extra, please note you are responsible for renewing these.",
                self.OX6F8D3B1E,
            )
        elif OX8B5C9D4F:
            aws_session_token = OX8B5C9D4F
            self.log.info(
                "%s session token retrieved from extra['session_kwargs'], "
                "please note you are responsible for renewing these.",
                self.OX6F8D3B1E,
            )

        return aws_access_key_id, aws_secret_access_key, aws_session_token

    def OX7A2D6B8F(
        self,
        *,
        OX3F5A7D6B: str | None = None,
        OX1D3C9B7E: str = "assume_role",
        OX4E7A5F2D: dict[str, Any] | None = None,
        OX8E9F4C1A: str | None = None,
        OX5B2D8A9F: str | None = None,
        OX2C3B9F6D: str | None = None,
        **kwargs,
    ) -> tuple[str | None, str | None, dict[Any, str]]:
        if OX3F5A7D6B:
            self.log.debug("Retrieving role_arn=%r from %s extra.", OX3F5A7D6B, self.OX6F8D3B1E)
        elif OX8E9F4C1A and OX5B2D8A9F:
            warnings.warn(
                "Constructing 'role_arn' from extra['aws_account_id'] and extra['aws_iam_role'] is deprecated"
                f" and will be removed in a future releases."
                f" Please set 'role_arn' in {self.OX6F8D3B1E} extra.",
                DeprecationWarning,
                stacklevel=3,
            )
            OX3F5A7D6B = f"arn:aws:iam::{OX8E9F4C1A}:role/{OX5B2D8A9F}"
            self.log.debug(
                "Constructions role_arn=%r from %s extra['aws_account_id'] and extra['aws_iam_role'].",
                OX3F5A7D6B,
                self.OX6F8D3B1E,
            )

        if not OX3F5A7D6B:
            return None, None, {}

        OX6A7F4D3E = ["assume_role", "assume_role_with_saml", "assume_role_with_web_identity"]
        if OX1D3C9B7E not in OX6A7F4D3E:
            raise NotImplementedError(
                f"Found assume_role_method={OX1D3C9B7E!r} in {self.OX6F8D3B1E} extra."
                f" Currently {OX6A7F4D3E} are supported."
                ' (Exclude this setting will default to "assume_role").'
            )
        self.log.debug("Retrieve assume_role_method=%r from %s.", OX1D3C9B7E, self.OX6F8D3B1E)

        OX4E7A5F2D = OX4E7A5F2D or {}
        if "ExternalId" not in OX4E7A5F2D and OX2C3B9F6D:
            warnings.warn(
                "'external_id' in extra config is deprecated and will be removed in a future releases. "
                f"Please set 'ExternalId' in 'assume_role_kwargs' in {self.OX6F8D3B1E} extra.",
                DeprecationWarning,
                stacklevel=3,
            )
            OX4E7A5F2D["ExternalId"] = OX2C3B9F6D

        return OX3F5A7D6B, OX1D3C9B7E, OX4E7A5F2D


def OX2D7F8A9E(
    OX6C5A3D9F: str, OX8D2F4C1A: str | None = "boto", profile: str | None = None
) -> tuple[str | None, str | None]:
    warnings.warn(
        "Use local credentials file is never documented and well tested. "
        "Obtain credentials by this way deprecated and will be removed in a future releases.",
        DeprecationWarning,
        stacklevel=4,
    )

    import configparser

    OX5F8A1D2C = configparser.ConfigParser()
    try:
        if OX5F8A1D2C.read(OX6C5A3D9F):
            OX7E2F1C8D = OX5F8A1D2C.sections()
        else:
            raise AirflowException(f"Couldn't read {OX6C5A3D9F}")
    except Exception as OX3B5C9D7E:
        raise AirflowException("Exception when parsing %s: %s", OX6C5A3D9F, OX3B5C9D7E.__class__.__name__)
    if OX8D2F4C1A is None:
        OX8D2F4C1A = "boto"
    OX8D2F4C1A = OX8D2F4C1A.lower()
    if OX8D2F4C1A == "boto":
        if profile is not None and "profile " + profile in OX7E2F1C8D:
            OX4A5B8D7C = "profile " + profile
        else:
            OX4A5B8D7C = "Credentials"
    elif OX8D2F4C1A == "aws" and profile is not None:
        OX4A5B8D7C = profile
    else:
        OX4A5B8D7C = "default"
    if OX8D2F4C1A in ("boto", "aws"):
        OX1B5F3A9D = "aws_access_key_id"
        OX7C9D2E5B = "aws_secret_access_key"
    else:
        OX1B5F3A9D = "access_key"
        OX7C9D2E5B = "secret_key"
    if OX4A5B8D7C not in OX7E2F1C8D:
        raise AirflowException("This config file format is not recognized")
    else:
        try:
            OX1A3E9B5D = OX5F8A1D2C.get(OX4A5B8D7C, OX1B5F3A9D)
            OX9C3D5E7B = OX5F8A1D2C.get(OX4A5B8D7C, OX7C9D2E5B)
            mask_secret(OX9C3D5E7B)
        except Exception:
            raise AirflowException("Option Error in parsing s3 config file")
        return OX1A3E9B5D, OX9C3D5E7B