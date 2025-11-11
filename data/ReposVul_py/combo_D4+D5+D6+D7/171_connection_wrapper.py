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
class _ConnMeta:
    conn_type: str | None = None
    conn_id: str | None = None
    description: str | None = None
    host: str | None = None
    login: str | None = None
    password: str | None = None
    schema: str | None = None
    port: int | None = None
    extra: str | dict | None = None

    @property
    def extra_dejson(self):
        if not self.extra:
            return {}
        x = deepcopy(self.extra)
        if isinstance(x, str):
            try:
                x = json.loads(x)
            except json.JSONDecodeError as err:
                raise AirflowException(f"'extra' expected valid JSON-Object string. Original error:\n * {err}") from None
        if not isinstance(x, dict):
            raise TypeError(f"Expected JSON-Object or dict, got {type(x).__name__}.")
        return x


@dataclass
class AwsConnectionWrapper(LoggingMixin):
    c: InitVar[Connection | AwsConnectionWrapper | _ConnMeta | None]
    r: str | None = field(default=None)
    b: Config | None = field(default=None)
    v: bool | str | None = field(default=None)
    t: str | ArgNotSet | None = field(init=False, default=NOTSET)
    n: str | None = field(init=False, default=None)
    l: str | None = field(init=False, repr=False, default=None)
    p: str | None = field(init=False, repr=False, default=None)
    s: str | None = field(init=False, repr=False, default=None)
    e: dict[str, Any] = field(init=False, repr=False, default_factory=dict)
    a1: str | None = field(init=False, default=None)
    a2: str | None = field(init=False, default=None)
    a3: str | None = field(init=False, default=None)
    q: str | None = field(init=False, default=None)
    u: str | None = field(init=False, default=None)
    z: str | None = field(init=False, default=None)
    am: str | None = field(init=False, default=None)
    ak: dict[str, Any] = field(init=False, default_factory=dict)

    @cached_property
    def conn_repr(self):
        return f"AWS Connection (conn_id={self.t!r}, conn_type={self.n!r})"

    def get_service_config(self, sn):
        return self.extra_dejson.get("service_config", {}).get(sn, {})

    def __post_init__(self, c: Connection):
        if isinstance(c, type(self)):
            for fl in fields(c):
                v = getattr(c, fl.name)
                if not fl.init:
                    setattr(self, fl.name, v)
                else:
                    if fl.default is not MISSING:
                        d = fl.default
                    elif fl.default_factory is not MISSING:
                        d = fl.default_factory()
                    else:
                        continue

                    ov = getattr(self, fl.name)
                    if ov == d:
                        setattr(self, fl.name, v)
            return
        elif not c:
            return

        self.t = c.conn_id
        self.n = c.conn_type or "aws"
        self.l = c.login
        self.p = c.password
        self.s = c.schema or None
        self.e = deepcopy(c.extra_dejson)

        if self.n.lower() == "s3":
            warnings.warn(
                f"{self.conn_repr} has connection type 's3', which has been replaced by connection type 'aws'. Please update your connection to have `conn_type='aws'`.",
                DeprecationWarning,
                stacklevel=2,
            )
        elif self.n != "aws":
            warnings.warn(
                f"{self.conn_repr} expected connection type 'aws', got {self.n!r}. This connection might not work correctly. Please use Amazon Web Services Connection type.",
                UserWarning,
                stacklevel=2,
            )

        x = deepcopy(c.extra_dejson)
        sk = x.get("session_kwargs", {})
        if sk:
            warnings.warn(
                "'session_kwargs' in extra config is deprecated and will be removed in a future releases. Please specify arguments passed to boto3 Session directly in {self.conn_repr} extra.",
                DeprecationWarning,
                stacklevel=2,
            )

        ic = self._get_credentials(**x)
        self.a1, self.a2, self.a3 = ic

        if not self.r:
            if "region_name" in x:
                self.r = x["region_name"]
                self.log.debug("Retrieving region_name=%s from %s extra.", self.r, self.conn_repr)
            elif "region_name" in sk:
                self.r = sk["region_name"]
                self.log.debug(
                    "Retrieving region_name=%s from %s extra['session_kwargs'].",
                    self.r,
                    self.conn_repr,
                )

        if self.v is None and "verify" in x:
            self.v = x["verify"]
            self.log.debug("Retrieving verify=%s from %s extra.", self.v, self.conn_repr)

        if "profile_name" in x:
            self.q = x["profile_name"]
            self.log.debug("Retrieving profile_name=%s from %s extra.", self.q, self.conn_repr)
        elif "profile_name" in sk:
            self.q = sk["profile_name"]
            self.log.debug(
                "Retrieving profile_name=%s from %s extra['session_kwargs'].",
                self.q,
                self.conn_repr,
            )

        if "profile" in x and "s3_config_file" not in x and not self.q:
            warnings.warn(
                f"Found 'profile' without specifying 's3_config_file' in {self.conn_repr} extra. If required profile from AWS Shared Credentials please set 'profile_name' in {self.conn_repr} extra.",
                UserWarning,
                stacklevel=2,
            )

        cw = x.get("config_kwargs")
        if not self.b and cw:
            self.log.debug("Retrieving botocore config=%s from %s extra.", cw, self.conn_repr)
            self.b = Config(**cw)

        if c.host:
            warnings.warn(
                f"Host {c.host} specified in the connection is not used. Please, set it on extra['endpoint_url'] instead",
                DeprecationWarning,
                stacklevel=2,
            )

        self.u = x.get("host")
        if self.u:
            warnings.warn(
                "extra['host'] is deprecated and will be removed in a future release. Please set extra['endpoint_url'] instead",
                DeprecationWarning,
                stacklevel=2,
            )
        else:
            self.u = x.get("endpoint_url")

        arc = self._get_assume_role_configs(**x)
        self.z, self.am, self.ak = arc

    @classmethod
    def from_connection_metadata(cls, t: str | None = None, l: str | None = None, p: str | None = None, e: dict[str, Any] | None = None):
        cm = _ConnMeta(conn_id=t, conn_type="aws", login=l, password=p, extra=e)
        return cls(c=cm)

    @property
    def extra_dejson(self):
        return self.e

    @property
    def session_kwargs(self) -> dict[str, Any]:
        return trim_none_values(
            {
                "aws_access_key_id": self.a1,
                "aws_secret_access_key": self.a2,
                "aws_session_token": self.a3,
                "region_name": self.r,
                "profile_name": self.q,
            }
        )

    def __bool__(self):
        return self.t is not NOTSET

    def _get_credentials(
        self,
        *,
        aws_access_key_id: str | None = None,
        aws_secret_access_key: str | None = None,
        aws_session_token: str | None = None,
        s3_config_file: str | None = None,
        s3_config_format: str | None = None,
        profile: str | None = None,
        session_kwargs: dict[str, Any] | None = None,
        **kwargs,
    ) -> tuple[str | None, str | None, str | None]:
        sk = session_kwargs or {}
        sak = sk.get("aws_access_key_id")
        ssk = sk.get("aws_secret_access_key")
        sst = sk.get("aws_session_token")

        if self.l and self.p:
            self.log.info("%s credentials retrieved from login and password.", self.conn_repr)
            aws_access_key_id, aws_secret_access_key = self.l, self.p
        elif aws_access_key_id and aws_secret_access_key:
            self.log.info("%s credentials retrieved from extra.", self.conn_repr)
        elif sak and ssk:
            aws_access_key_id = sak
            aws_secret_access_key = ssk
            self.log.info("%s credentials retrieved from extra['session_kwargs'].", self.conn_repr)
        elif s3_config_file:
            aws_access_key_id, aws_secret_access_key = _parse_s3_config(
                s3_config_file,
                s3_config_format,
                profile,
            )
            self.log.info("%s credentials retrieved from extra['s3_config_file']", self.conn_repr)

        if aws_session_token:
            self.log.info(
                "%s session token retrieved from extra, please note you are responsible for renewing these.",
                self.conn_repr,
            )
        elif sst:
            aws_session_token = sst
            self.log.info(
                "%s session token retrieved from extra['session_kwargs'], please note you are responsible for renewing these.",
                self.conn_repr,
            )

        return aws_access_key_id, aws_secret_access_key, aws_session_token

    def _get_assume_role_configs(
        self,
        *,
        role_arn: str | None = None,
        assume_role_method: str = "assume_role",
        assume_role_kwargs: dict[str, Any] | None = None,
        aws_account_id: str | None = None,
        aws_iam_role: str | None = None,
        external_id: str | None = None,
        **kwargs,
    ) -> tuple[str | None, str | None, dict[Any, str]]:
        if role_arn:
            self.log.debug("Retrieving role_arn=%r from %s extra.", role_arn, self.conn_repr)
        elif aws_account_id and aws_iam_role:
            warnings.warn(
                "Constructing 'role_arn' from extra['aws_account_id'] and extra['aws_iam_role'] is deprecated and will be removed in a future releases. Please set 'role_arn' in {self.conn_repr} extra.",
                DeprecationWarning,
                stacklevel=3,
            )
            role_arn = f"arn:aws:iam::{aws_account_id}:role/{aws_iam_role}"
            self.log.debug(
                "Constructions role_arn=%r from %s extra['aws_account_id'] and extra['aws_iam_role'].",
                role_arn,
                self.conn_repr,
            )

        if not role_arn:
            return None, None, {}

        sm = ["assume_role", "assume_role_with_saml", "assume_role_with_web_identity"]
        if assume_role_method not in sm:
            raise NotImplementedError(
                f"Found assume_role_method={assume_role_method!r} in {self.conn_repr} extra. Currently {sm} are supported. (Exclude this setting will default to 'assume_role')."
            )
        self.log.debug("Retrieve assume_role_method=%r from %s.", assume_role_method, self.conn_repr)

        assume_role_kwargs = assume_role_kwargs or {}
        if "ExternalId" not in assume_role_kwargs and external_id:
            warnings.warn(
                "'external_id' in extra config is deprecated and will be removed in a future releases. Please set 'ExternalId' in 'assume_role_kwargs' in {self.conn_repr} extra.",
                DeprecationWarning,
                stacklevel=3,
            )
            assume_role_kwargs["ExternalId"] = external_id

        return role_arn, assume_role_method, assume_role_kwargs


def _parse_s3_config(
    config_file_name: str, config_format: str | None = "boto", profile: str | None = None
) -> tuple[str | None, str | None]:
    warnings.warn(
        "Use local credentials file is never documented and well tested. Obtain credentials by this way deprecated and will be removed in a future releases.",
        DeprecationWarning,
        stacklevel=4,
    )

    import configparser

    c = configparser.ConfigParser()
    try:
        if c.read(config_file_name):
            s = c.sections()
        else:
            raise AirflowException(f"Couldn't read {config_file_name}")
    except Exception as e:
        raise AirflowException("Exception when parsing %s: %s", config_file_name, e.__class__.__name__)

    if config_format is None:
        config_format = "boto"
    cf = config_format.lower()
    if cf == "boto":
        if profile is not None and "profile " + profile in s:
            cs = "profile " + profile
        else:
            cs = "Credentials"
    elif cf == "aws" and profile is not None:
        cs = profile
    else:
        cs = "default"

    if cf in ("boto", "aws"):
        ko = "aws_access_key_id"
        so = "aws_secret_access_key"
    else:
        ko = "access_key"
        so = "secret_key"

    if cs not in s:
        raise AirflowException("This config file format is not recognized")
    else:
        try:
            ak = c.get(cs, ko)
            sk = c.get(cs, so)
            mask_secret(sk)
        except Exception:
            raise AirflowException("Option Error in parsing s3 config file")
        return ak, sk