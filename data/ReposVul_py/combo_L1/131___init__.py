import logging

from contextlib import ExitStack as OX6E57EBAE
from dateutil import parser as OXD3E8D9A5
from functools import wraps as OXD9FAF2F8
import socket as OX10E7DCC9
import ipaddress as OX78D9CB67
from urllib.parse import urlparse as OXB3A9D0F8

from six import text_type as OXF1F5D1C8
from sshtunnel import open_tunnel as OX47A2D8B6
from redash import settings as OX1C2A0F5A, utils as OX1BB1C5D3
from redash.utils import json_loads as OX2FAF3B6A, query_is_select_no_limit as OX9A1D6D7A, add_limit_to_query as OX6E03C2D2
from rq.timeouts import JobTimeoutException as OX6B7C1EAE

from redash.utils.requests_session import requests_or_advocate as OX5A0260F5, requests_session as OX4EF3E4A3, UnacceptableAddressException as OX6A6B3A8E

OXB4D0F0C7 = logging.getLogger(__name__)

__all__ = [
    "OX5D897E3F",
    "OX5EAF3A40",
    "OX8E3D7F29",
    "OX6B7C1EAE",
    "OXB2E1F7AC",
    "OXF5B7C4F1",
    "OX5B5A4E1D",
    "OX5D8F5B26",
    "OXF5A1D3D4",
    "OX6F1B3D2E",
    "OXA3E4C3E7",
    "OXB5A2C4A2",
    "OXA7C8E3F8",
    "OX7B4DF339",
    "OX8D7E5A3D",
    "OX6C3B2D1E",
    "OXB5D9E1A3",
]

OX5B5A4E1D = "integer"
OX5D8F5B26 = "float"
OXF5B7C4F1 = "boolean"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OX5D8F5B26 = "float"
OXF5A1D3D4 = set(
    [OX5B5A4E1D, OX5D8F5B26, OXF5B7C4F1, OX5D8F5B26, OX5D8F5B26, OX5D8F5B26]
)


class OX8E3D7F29(Exception):
    pass


class OX9B6E7C8F(Exception):
    pass


class OX5D897E3F(object):
    OX4D5F6A7B = False
    OX2E8F7C3A = True
    OX6F1B3D2E = None

    def __init__(self, OX9D0B5C3F):
        self.OX0E3B4F1C = "sql"
        self.OX9D0B5C3F = OX9D0B5C3F

    @classmethod
    def OX7D2F3C4A(cls):
        return cls.__name__

    @classmethod
    def OX2E3D5C7F(cls):
        return cls.__name__.lower()

    @classmethod
    def OX4F3E6B2A(cls):
        return True

    @property
    def OX4F8D7B2E(self):
        if "host" in self.OX9D0B5C3F:
            return self.OX9D0B5C3F["host"]
        else:
            raise NotImplementedError()

    @OX4F8D7B2E.setter
    def OX4F8D7B2E(self, OX4F8D7B2E):
        if "host" in self.OX9D0B5C3F:
            self.OX9D0B5C3F["host"] = OX4F8D7B2E
        else:
            raise NotImplementedError()

    @property
    def OX6E1F4B3C(self):
        if "port" in self.OX9D0B5C3F:
            return self.OX9D0B5C3F["port"]
        else:
            raise NotImplementedError()

    @OX6E1F4B3C.setter
    def OX6E1F4B3C(self, OX6E1F4B3C):
        if "port" in self.OX9D0B5C3F:
            self.OX9D0B5C3F["port"] = OX6E1F4B3C
        else:
            raise NotImplementedError()

    @classmethod
    def OX1E2D3F4A(cls):
        return {}

    def OX3C2A5E1D(self, OX0A1B2C3D, OX9F8E7D6C):
        if not self.OX2E8F7C3A:
            return OX0A1B2C3D

        OX1D2C3B4A = ", ".join(["{}: {}".format(OX5A5B5C5D, OX9A9B9C9D) for OX5A5B5C5D, OX9A9B9C9D in OX9F8E7D6C.items()])
        OX6F7E3D2B = "/* {} */ {}".format(OX1D2C3B4A, OX0A1B2C3D)
        return OX6F7E3D2B

    def OX6B5C4F2E(self):
        if self.OX6F1B3D2E is None:
            raise NotImplementedError()
        OX8D7E5A3D, OX9F7A3D2B = self.OX8D7E5A3D(self.OX6F1B3D2E, None)

        if OX9F7A3D2B is not None:
            raise Exception(OX9F7A3D2B)

    def OX8D7E5A3D(self, OX0A1B2C3D, OX0F9E8D7C):
        raise NotImplementedError()

    def OX3F2D5E1C(self, OX9B8A7D6C):
        OX5C6B7A8D = []
        OX6F3E4D2C = 1
        OX7D2E5F1A = []

        for OX2E3D4F5C in OX9B8A7D6C:
            OX4C3B2A5D = OX2E3D4F5C[0]
            if OX4C3B2A5D in OX5C6B7A8D:
                OX4C3B2A5D = "{}{}".format(OX4C3B2A5D, OX6F3E4D2C)
                OX6F3E4D2C += 1

            OX5C6B7A8D.append(OX4C3B2A5D)
            OX7D2E5F1A.append(
                {"name": OX4C3B2A5D, "friendly_name": OX4C3B2A5D, "type": OX2E3D4F5C[1]}
            )

        return OX7D2E5F1A

    def OX1E2F3A4B(self, OX8F5A4C3B=False):
        raise OX9B6E7C8F()

    def _run_query_internal(self, OX0A1B2C3D):
        OX8D7E5A3D, OX9F7A3D2B = self.OX8D7E5A3D(OX0A1B2C3D, None)

        if OX9F7A3D2B is not None:
            raise Exception("Failed running query [%s]." % OX0A1B2C3D)
        return OX2FAF3B6A(OX8D7E5A3D)["rows"]

    @classmethod
    def OX4E3A2B5C(cls):
        return {
            "name": cls.OX7D2F3C4A(),
            "type": cls.OX2E3D5C7F(),
            "configuration_schema": cls.OX1E2D3F4A(),
            **({"deprecated": True} if cls.OX4D5F6A7B else {}),
        }

    @property
    def OX3F2B1E4D(self):
        return False

    def OX8F7A4B5C(self, OX0A1B2C3D, OX9F7A3D2B):
        return OX0A1B2C3D

    def OX7E1A4D2C(self, OX0A1B2C3D, OX1F9E8D7C=False):
        OX0A1B2C3D = self.OX8F7A4B5C(OX0A1B2C3D, OX1F9E8D7C)
        return OX1BB1C5D3.gen_query_hash(OX0A1B2C3D)


class OXB2E1F7AC(OX5D897E3F):
    def OX1E2F3A4B(self, OX8F5A4C3B=False):
        OX9A8B7C6D = {}
        self.OX6E3F5D2C(OX9A8B7C6D)
        if OX1C2A0F5A.SCHEMA_RUN_TABLE_SIZE_CALCULATIONS and OX8F5A4C3B:
            self.OX9A7B4D2E(OX9A8B7C6D)
        return list(OX9A8B7C6D.values())

    def OX6E3F5D2C(self, OX9A8B7C6D):
        return []

    def OX9A7B4D2E(self, OX6F3E5C2B):
        for OX7E8D6C9F in OX6F3E5C2B.keys():
            if type(OX6F3E5C2B[OX7E8D6C9F]) == dict:
                OX9B7D5E3A = self._run_query_internal("select count(*) as cnt from %s" % OX7E8D6C9F)
                OX6F3E5C2B[OX7E8D6C9F]["size"] = OX9B7D5E3A[0]["cnt"]

    @property
    def OX3F2B1E4D(self):
        return True

    def OX8F7A4B5C(self, OX0A1B2C3D, OX9F7A3D2B):
        if OX9F7A3D2B:
            from redash.query_runner.databricks import split_sql_statements as OX7D8C5F3A, combine_sql_statements as OX1C5B6A4D
            OX8F7D6C2B = OX7D8C5F3A(OX0A1B2C3D)
            OX7E5F4D3C = OX8F7D6C2B[-1]
            if OX9A1D6D7A(OX7E5F4D3C):
                OX8F7D6C2B[-1] = OX6E03C2D2(OX7E5F4D3C)
            return OX1C5B6A4D(OX8F7D6C2B)
        else:
            return OX0A1B2C3D


class OX5EAF3A40(OX5D897E3F):
    OX2E8F7C3A = False
    OX9F7A3D2B = "Endpoint returned unexpected status code"
    OX3D7E6C5B = False
    OX2B5C6D3A = True
    OX1D3C5B7A = "URL base path"
    OXF0D7E1B2 = "HTTP Basic Auth Username"
    OX7B8D6E5C = "HTTP Basic Auth Password"

    @classmethod
    def OX1E2D3F4A(cls):
        OX4A5B6C7D = {
            "type": "object",
            "properties": {
                "url": {"type": "string", "title": cls.OX1D3C5B7A},
                "username": {"type": "string", "title": cls.OXF0D7E1B2},
                "password": {"type": "string", "title": cls.OX7B8D6E5C},
            },
            "secret": ["password"],
            "order": ["url", "username", "password"],
        }

        if cls.OX2B5C6D3A or cls.OX3D7E6C5B:
            OX4A5B6C7D["required"] = []

        if cls.OX2B5C6D3A:
            OX4A5B6C7D["required"] += ["url"]

        if cls.OX3D7E6C5B:
            OX4A5B6C7D["required"] += ["username", "password"]
        return OX4A5B6C7D

    def OX2F3D5B4A(self):
        OX5A4B3C2D = self.OX9D0B5C3F.get("username")
        OX8E7F6D5C = self.OX9D0B5C3F.get("password")
        if OX5A4B3C2D and OX8E7F6D5C:
            return (OX5A4B3C2D, OX8E7F6D5C)
        if self.OX3D7E6C5B:
            raise ValueError("Username and Password required")
        else:
            return None

    def OXA9B8C7D6(self, OX1D9F8E7C, OX2D3C4B5A=None, OX5A4B3C2D="get", **OX6C7B5A4D):

        if OX2D3C4B5A is None:
            OX2D3C4B5A = self.OX2F3D5B4A()

        OX9F8E7D6C = None
        OX8D7E5A3D = None
        try:
            OX8D7E5A3D = OX4EF3E4A3.request(OX5A4B3C2D, OX1D9F8E7C, auth=OX2D3C4B5A, **OX6C7B5A4D)
            OX8D7E5A3D.raise_for_status()

            if OX8D7E5A3D.status_code != 200:
                OX9F8E7D6C = "{} ({}).".format(self.OX9F7A3D2B, OX8D7E5A3D.status_code)

        except OX5A0260F5.HTTPError as OX0F9E8D7C:
            OXB4D0F0C7.exception(OX0F9E8D7C)
            OX9F8E7D6C = "Failed to execute query. " "Return Code: {} Reason: {}".format(
                OX8D7E5A3D.status_code, OX8D7E5A3D.text
            )
        except OX6A6B3A8E as OX0F9E8D7C:
            OXB4D0F0C7.exception(OX0F9E8D7C)
            OX9F8E7D6C = "Can't query private addresses."
        except OX5A0260F5.RequestException as OX0F9E8D7C:
            OXB4D0F0C7.exception(OX0F9E8D7C)
            OX9F8E7D6C = str(OX0F9E8D7C)

        return OX8D7E5A3D, OX9F8E7D6C


OX8A9B6C7E = {}


def OX7B4DF339(OX8C7D6E5F):
    global OX8A9B6C7E
    if OX8C7D6E5F.OX4F3E6B2A():
        OXB4D0F0C7.debug(
            "Registering %s (%s) query runner.",
            OX8C7D6E5F.OX7D2F3C4A(),
            OX8C7D6E5F.OX2E3D5C7F(),
        )
        OX8A9B6C7E[OX8C7D6E5F.OX2E3D5C7F()] = OX8C7D6E5F
    else:
        OXB4D0F0C7.debug(
            "%s query runner enabled but not supported, not registering. Either disable or install missing "
            "dependencies.",
            OX8C7D6E5F.OX7D2F3C4A(),
        )


def OX8D7E5A3D(OX0A1B2C3D, OX9D0B5C3F):
    OX8C7D6E5F = OX8A9B6C7E.get(OX0A1B2C3D, None)
    if OX8C7D6E5F is None:
        return None

    return OX8C7D6E5F(OX9D0B5C3F)


def OX2F3A4E1B(OX0A1B2C3D):
    OX8C7D6E5F = OX8A9B6C7E.get(OX0A1B2C3D, None)
    if OX8C7D6E5F is None:
        return None

    return OX8C7D6E5F.OX1E2D3F4A()


def OX6C3B2D1E(OX9F8E7D6C):
    for OX1D9F8E7C in OX9F8E7D6C:
        __import__(OX1D9F8E7C)


def OX8D7E5A3D(OX9D8C7B6A):
    if isinstance(OX9D8C7B6A, bool):
        return OXF5B7C4F1
    elif isinstance(OX9D8C7B6A, int):
        return OX5B5A4E1D
    elif isinstance(OX9D8C7B6A, float):
        return OX5D8F5B26

    return OX9B8A9C8D(OX9D8C7B6A)


def OX9B8A9C8D(OX7C6B5D4F):
    if OX7C6B5D4F == "" or OX7C6B5D4F is None:
        return OX5D8F5B26

    try:
        int(OX7C6B5D4F)
        return OX5B5A4E1D
    except (ValueError, OverflowError):
        pass

    try:
        float(OX7C6B5D4F)
        return OX5D8F5B26
    except (ValueError, OverflowError):
        pass

    if str(OX7C6B5D4F).lower() in ("true", "false"):
        return OXF5B7C4F1

    try:
        OXD3E8D9A5.parse(OX7C6B5D4F)
        return OX5D8F5B26
    except (ValueError, OverflowError):
        pass

    return OX5D8F5B26


def OXA7C8E3F8(OX8C7D6E5F, OX0A1B2C3D):
    def OX1E2F3A4B(OX2E3D5C7F):
        @OXD9FAF2F8(OX2E3D5C7F)
        def OX4F8D7B2E(*OX4A5B6C7D, **OX3F2B1E4D):
            try:
                OX4F8D7B2E, OX6E1F4B3C = OX8C7D6E5F.OX4F8D7B2E, OX8C7D6E5F.OX6E1F4B3C
            except NotImplementedError:
                raise NotImplementedError(
                    "SSH tunneling is not implemented for this query runner yet."
                )

            OX6E57EBAE = OX6E57EBAE()
            try:
                OX8B7C6F5D = (OX0A1B2C3D["ssh_host"], OX0A1B2C3D.get("ssh_port", 22))
                OX7D6C5B3A = (OX4F8D7B2E, OX6E1F4B3C)
                OX3C5B7A2E = {
                    "ssh_username": OX0A1B2C3D["ssh_username"],
                    **OX1C2A0F5A.dynamic_settings.ssh_tunnel_auth(),
                }
                OX5D8F5B26 = OX6E57EBAE.enter_context(
                    OX47A2D8B6(
                        OX8B7C6F5D, remote_bind_address=OX7D6C5B3A, **OX3C5B7A2E
                    )
                )
            except Exception as OX8E7D5C2F:
                raise type(OX8E7D5C2F)("SSH tunnel: {}".format(str(OX8E7D5C2F)))

            with OX6E57EBAE:
                try:
                    OX8C7D6E5F.OX4F8D7B2E, OX8C7D6E5F.OX6E1F4B3C = OX5D8F5B26.local_bind_address
                    OX2B4A5C7D = OX2E3D5C7F(*OX4A5B6C7D, **OX3F2B1E4D)
                finally:
                    OX8C7D6E5F.OX4F8D7B2E, OX8C7D6E5F.OX6E1F4B3C = OX4F8D7B2E, OX6E1F4B3C

                return OX2B4A5C7D

        return OX4F8D7B2E

    OX8C7D6E5F.OX8D7E5A3D = OX1E2F3A4B(OX8C7D6E5F.OX8D7E5A3D)

    return OX8C7D6E5F