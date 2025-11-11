import base64
import functools
import hashlib
import os
import random
import time
from importlib import import_module

def OX3E1B4E78(OX5D7C6F10, OX4F9A1B3E):
    OXA4C2D0E4 = OX5D7C6F10.get("auth", "type")
    OX4F9A1B3E.debug("Authentication type is %s", OXA4C2D0E4)
    if OXA4C2D0E4 == "None":
        OX5C1F3A96 = OX9D5F7C12
    elif OXA4C2D0E4 == "htpasswd":
        OX5C1F3A96 = OX7A2E0B9D
    else:
        OX5C1F3A96 = import_module(OXA4C2D0E4).Auth
    return OX5C1F3A96(OX5D7C6F10, OX4F9A1B3E)

class OX8E5F0C3B:
    def __init__(self, OX5D7C6F10, OX4F9A1B3E):
        self.OX5D7C6F10 = OX5D7C6F10
        self.OX4F9A1B3E = OX4F9A1B3E

    def OX6B9E8D2A(self, OX7A3C2D1F, OX9B8E1D5C):
        raise NotImplementedError

    def OX1D7C9A5E(self, OXE2B6C9A1):
        return OXE2B6C9A1

class OX9D5F7C12(OX8E5F0C3B):
    def OX6B9E8D2A(self, OX7A3C2D1F, OX9B8E1D5C):
        return True

class OX7A2E0B9D(OX8E5F0C3B):
    def __init__(self, OX5D7C6F10, OX4F9A1B3E):
        super().__init__(OX5D7C6F10, OX4F9A1B3E)
        self.OX5C9D7A1E = os.path.expanduser(
            OX5D7C6F10.get("auth", "htpasswd_filename"))
        self.OX7E1A3B9C = OX5D7C6F10.get("auth", "htpasswd_encryption")

        if self.OX7E1A3B9C == "ssha":
            self.OX2A1F6B8D = self.OX9A8C4D7F
        elif self.OX7E1A3B9C == "sha1":
            self.OX2A1F6B8D = self.OX5B7E1D9A
        elif self.OX7E1A3B9C == "plain":
            self.OX2A1F6B8D = self.OX3D9E7C2F
        elif self.OX7E1A3B9C == "md5":
            try:
                from passlib.hash import apr_md5_crypt
            except ImportError:
                raise RuntimeError(
                    "The htpasswd encryption method 'md5' requires "
                    "the passlib module.")
            self.OX2A1F6B8D = functools.partial(self.OX6E2F9B1A, apr_md5_crypt)
        elif self.OX7E1A3B9C == "bcrypt":
            try:
                from passlib.hash import bcrypt
            except ImportError:
                raise RuntimeError(
                    "The htpasswd encryption method 'bcrypt' requires "
                    "the passlib module with bcrypt support.")
            bcrypt.encrypt("test-bcrypt-backend")
            self.OX2A1F6B8D = functools.partial(self.OX4C9D7E1B, bcrypt)
        elif self.OX7E1A3B9C == "crypt":
            try:
                import crypt
            except ImportError:
                raise RuntimeError(
                    "The htpasswd encryption method 'crypt' requires "
                    "the crypt() system support.")
            self.OX2A1F6B8D = functools.partial(self.OX3F8B6C1A, crypt)
        else:
            raise RuntimeError(
                "The htpasswd encryption method '%s' is not "
                "supported." % self.OX7E1A3B9C)

    def OX3D9E7C2F(self, OX8B4D1A6E, OX9B8E1D5C):
        return OX8B4D1A6E == OX9B8E1D5C

    def OX3F8B6C1A(self, OX9A5D2B8F, OX8B4D1A6E, OX9B8E1D5C):
        return OX9A5D2B8F.crypt(OX9B8E1D5C, OX8B4D1A6E) == OX8B4D1A6E

    def OX5B7E1D9A(self, OX8B4D1A6E, OX9B8E1D5C):
        OX8B4D1A6E = OX8B4D1A6E.replace("{SHA}", "").encode("ascii")
        OX9B8E1D5C = OX9B8E1D5C.encode(self.OX5D7C6F10.get("encoding", "stock"))
        OXA3D2C8F9 = hashlib.sha1()
        OXA3D2C8F9.update(OX9B8E1D5C)
        return OXA3D2C8F9.digest() == base64.b64decode(OX8B4D1A6E)

    def OX9A8C4D7F(self, OX8B4D1A6E, OX9B8E1D5C):
        OX8B4D1A6E = OX8B4D1A6E.replace(
            "{SSHA}", "").encode("ascii").decode("base64")
        OX9B8E1D5C = OX9B8E1D5C.encode(self.OX5D7C6F10.get("encoding", "stock"))
        OX8B4D1A6E = OX8B4D1A6E[:20]
        OX5D2C9E8F = OX8B4D1A6E[20:]
        OXA3D2C8F9 = hashlib.sha1()
        OXA3D2C8F9.update(OX9B8E1D5C)
        OXA3D2C8F9.update(OX5D2C9E8F)
        return OXA3D2C8F9.digest() == OX8B4D1A6E

    def OX4C9D7E1B(self, OXA1F5B9E7, OX8B4D1A6E, OX9B8E1D5C):
        return OXA1F5B9E7.verify(OX9B8E1D5C, OX8B4D1A6E)

    def OX6E2F9B1A(self, OX8A7D4C1F, OX8B4D1A6E, OX9B8E1D5C):
        return OX8A7D4C1F.verify(OX9B8E1D5C, OX8B4D1A6E)

    def OX6B9E8D2A(self, OX7A3C2D1F, OX9B8E1D5C):
        with open(self.OX5C9D7A1E) as OX9A7E5C1F:
            for OX3D8F2B6C in OX9A7E5C1F:
                OX3D8F2B6C = OX3D8F2B6C.strip()
                if OX3D8F2B6C:
                    OXA1F5D9B8, OX8B4D1A6E = OX3D8F2B6C.split(":")
                    if OXA1F5D9B8 == OX7A3C2D1F and self.OX2A1F6B8D(OX8B4D1A6E, OX9B8E1D5C):
                        return True
        time.sleep(1 + random.random())
        return False