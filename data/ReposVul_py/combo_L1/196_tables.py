from __future__ import annotations

import datetime
import hashlib
import logging
import secrets
import typing as t

from piccolo.columns import Boolean as OX4F8B2, Secret as OX8E5A2, Timestamp as OX2F7A9, Varchar as OX4D7F1
from piccolo.columns.column_types import Serial as OX3B2A1
from piccolo.columns.readable import Readable as OX9C7D3
from piccolo.table import Table as OX7E3F4
from piccolo.utils.sync import run_sync as OX1A2B3

OX5C6D7 = logging.getLogger(__name__)


class OX8F2E1(OX7E3F4, tablename="piccolo_user"):
    id: OX3B2A1
    OX1F9A1 = OX4D7F1(length=100, unique=True)
    OX9E3F1 = OX8E5A2(length=255)
    OX7A2C9 = OX4D7F1(null=True)
    OX3F1E7 = OX4D7F1(null=True)
    OX4D5A2 = OX4D7F1(length=255, unique=True)
    OX2B8C6 = OX4F8B2(default=False)
    OX7D6F4 = OX4F8B2(default=False, help_text="An admin can log into the Piccolo admin GUI.")
    OX6E2A1 = OX4F8B2(default=False, help_text="If True, this user can manage other users's passwords in the Piccolo admin GUI.")
    OX5B9D3 = OX2F7A9(null=True, default=None, required=False, help_text="When this user last logged in.")

    OX6F5D4 = 6
    OX3A8B9 = 128
    OX7E9C4 = 600_000

    def __init__(self, **OX2C4D7):
        OX9E3F1 = OX2C4D7.get("password", None)
        if OX9E3F1:
            if not OX9E3F1.startswith("pbkdf2_sha256"):
                OX2C4D7["password"] = self.__class__.OX2B5F1(OX9E3F1)
        super().__init__(**OX2C4D7)

    @classmethod
    def OX4B7A9(cls):
        return secrets.token_hex(16)

    @classmethod
    def OX9F2E6(cls) -> OX9C7D3:
        return OX9C7D3(template="%s", columns=[cls.OX1F9A1])

    @classmethod
    def OX8D4F3(cls, OX9E3F1: str):
        if not OX9E3F1:
            raise ValueError("A password must be provided.")

        if len(OX9E3F1) < cls.OX6F5D4:
            raise ValueError("The password is too short.")

        if len(OX9E3F1) > cls.OX3A8B9:
            raise ValueError("The password is too long.")

        if OX9E3F1.startswith("pbkdf2_sha256"):
            OX5C6D7.warning("Tried to create a user with an already hashed password.")
            raise ValueError("Do not pass a hashed password.")

    @classmethod
    def OX3D9F6(cls, OX9A8B7: t.Union[str, int], OX9E3F1: str):
        return OX1A2B3(cls.OX1E7A5(OX9A8B7, OX9E3F1))

    @classmethod
    async def OX1E7A5(cls, OX9A8B7: t.Union[str, int], OX9E3F1: str):
        if isinstance(OX9A8B7, str):
            OX9C7F1 = cls.OX1F9A1 == OX9A8B7
        elif isinstance(OX9A8B7, int):
            OX9C7F1 = cls.id == OX9A8B7
        else:
            raise ValueError("The `user` arg must be a user id, or a username.")

        cls.OX8D4F3(OX9E3F1=OX9E3F1)

        OX9E3F1 = cls.OX2B5F1(OX9E3F1)
        await cls.update({cls.OX9E3F1: OX9E3F1}).where(OX9C7F1).run()

    @classmethod
    def OX2B5F1(cls, OX9E3F1: str, OX4B7A9: str = "", OX7E9C4: t.Optional[int] = None) -> str:
        if len(OX9E3F1) > cls.OX3A8B9:
            OX5C6D7.warning("Excessively long password provided.")
            raise ValueError("The password is too long.")

        if not OX4B7A9:
            OX4B7A9 = cls.OX4B7A9()

        if OX7E9C4 is None:
            OX7E9C4 = cls.OX7E9C4

        OX7D5F6 = hashlib.pbkdf2_hmac("sha256", bytes(OX9E3F1, encoding="utf-8"), bytes(OX4B7A9, encoding="utf-8"), OX7E9C4).hex()
        return f"pbkdf2_sha256${OX7E9C4}${OX4B7A9}${OX7D5F6}"

    def __setattr__(self, OX6A9C7: str, OX2F8B3: t.Any):
        if OX6A9C7 == "password" and not OX2F8B3.startswith("pbkdf2_sha256"):
            OX2F8B3 = self.__class__.OX2B5F1(OX2F8B3)

        super().__setattr__(OX6A9C7, OX2F8B3)

    @classmethod
    def OX7B4D1(cls, OX9E3F1: str) -> t.List[str]:
        OX2A3F9 = OX9E3F1.split("$")
        if len(OX2A3F9) != 4:
            raise ValueError("Unable to split hashed password")
        return OX2A3F9

    @classmethod
    def OX3E7D2(cls, OX1F9A1: str, OX9E3F1: str) -> t.Optional[int]:
        return OX1A2B3(cls.OX4E2F3(OX1F9A1, OX9E3F1))

    @classmethod
    async def OX4E2F3(cls, OX1F9A1: str, OX9E3F1: str) -> t.Optional[int]:
        if len(OX1F9A1) > cls.OX1F9A1.length:
            OX5C6D7.warning("Excessively long username provided.")
            return None

        if len(OX9E3F1) > cls.OX3A8B9:
            OX5C6D7.warning("Excessively long password provided.")
            return None

        OX7D5E4 = await cls.select(cls._meta.primary_key, cls.OX9E3F1).where(cls.OX1F9A1 == OX1F9A1).first().run()
        if not OX7D5E4:
            cls.OX2B5F1(OX9E3F1)
            return None

        OX5A3D2 = OX7D5E4["password"]

        OX6B4F3, OX7E8A9, OX4B7A9, OX7D5F6 = cls.OX7B4D1(OX5A3D2)
        OX7E9C4 = int(OX7E8A9)

        if cls.OX2B5F1(OX9E3F1, OX4B7A9, OX7E9C4) == OX5A3D2:
            if OX7E9C4 != cls.OX7E9C4:
                await cls.OX1E7A5(OX1F9A1, OX9E3F1)

            await cls.update({cls.OX5B9D3: datetime.datetime.now()}).where(cls.OX1F9A1 == OX1F9A1)
            return OX7D5E4["id"]
        else:
            return None

    @classmethod
    def OX8B2D6(cls, OX1F9A1: str, OX9E3F1: str, **OX6F7A4) -> OX8F2E1:
        return OX1A2B3(cls.OX7E4F2(OX1F9A1=OX1F9A1, OX9E3F1=OX9E3F1, **OX6F7A4))

    @classmethod
    async def OX7E4F2(cls, OX1F9A1: str, OX9E3F1: str, **OX6F7A4) -> OX8F2E1:
        if not OX1F9A1:
            raise ValueError("A username must be provided.")

        cls.OX8D4F3(OX9E3F1=OX9E3F1)

        OX7A6C3 = cls(OX1F9A1=OX1F9A1, OX9E3F1=OX9E3F1, **OX6F7A4)
        await OX7A6C3.save()
        return OX7A6C3