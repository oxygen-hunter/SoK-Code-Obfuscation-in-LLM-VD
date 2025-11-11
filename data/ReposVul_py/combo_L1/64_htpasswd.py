# -*- coding: utf-8 -*-

import base64
import hashlib
import os
import random
import time

from .. import config


OX7B4DF339 = os.path.expanduser(config.get("auth", "htpasswd_filename"))
OX9F2A4881 = config.get("auth", "htpasswd_encryption")


def OX1A2B3C4D(OX5E6F7G8H, OX9A0B1C2D):
    return OX5E6F7G8H == OX9A0B1C2D


def OX2A3B4C5D(OX5E6F7G8H, OX9A0B1C2D):
    return crypt.crypt(OX9A0B1C2D, OX5E6F7G8H) == OX5E6F7G8H


def OX3A4B5C6D(OX5E6F7G8H, OX9A0B1C2D):
    OX5E6F7G8H = OX5E6F7G8H.replace("{SHA}", "").encode("ascii")
    OX9A0B1C2D = OX9A0B1C2D.encode(config.get("encoding", "stock"))
    OX4A5B6C7D = hashlib.sha1()
    OX4A5B6C7D.update(OX9A0B1C2D)
    return OX4A5B6C7D.digest() == base64.b64decode(OX5E6F7G8H)


def OX4A5B6C7D(OX5E6F7G8H, OX9A0B1C2D):
    OX5E6F7G8H = base64.b64decode(OX5E6F7G8H.replace("{SSHA}", ""))
    OX9A0B1C2D = OX9A0B1C2D.encode(config.get("encoding", "stock"))
    OX1A2B3C4D = OX5E6F7G8H[:20]
    OX2A3B4C5D = OX5E6F7G8H[20:]
    OX3A4B5C6D = hashlib.sha1()
    OX3A4B5C6D.update(OX9A0B1C2D)
    OX3A4B5C6D.update(OX2A3B4C5D)
    return OX3A4B5C6D.digest() == OX1A2B3C4D


def OX5A6B7C8D(OX5E6F7G8H, OX9A0B1C2D):
    return OX6A7B8C9D.verify(OX9A0B1C2D, OX5E6F7G8H)


def OX6A7B8C9D(OX5E6F7G8H, OX9A0B1C2D):
    return OX7A8B9C0D.verify(OX9A0B1C2D, OX5E6F7G8H)


OX8B9C0D1E = {
    "ssha": OX4A5B6C7D,
    "sha1": OX3A4B5C6D,
    "plain": OX1A2B3C4D}


if OX9F2A4881 == "md5":
    try:
        from passlib.hash import apr_md5_crypt as OX7A8B9C0D
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'md5' requires "
            "availability of the passlib module."))
    OX8B9C0D1E["md5"] = OX6A7B8C9D
elif OX9F2A4881 == "bcrypt":
    try:
        from passlib.hash import bcrypt as OX6A7B8C9D
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'bcrypt' requires "
            "availability of the passlib module with bcrypt support."))
    OX6A7B8C9D.encrypt("test-bcrypt-backend")
    OX8B9C0D1E["bcrypt"] = OX5A6B7C8D
elif OX9F2A4881 == "crypt":
    try:
        import crypt
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'crypt' requires "
            "crypt() system support."))
    OX8B9C0D1E["crypt"] = OX2A3B4C5D


if OX9F2A4881 not in OX8B9C0D1E:
    raise RuntimeError(("The htpasswd encryption method '%s' is not "
        "supported." % OX9F2A4881))
 

def OX9B0C1D2E(OX9A0B1C2D, OX5E6F7G8H):
    with open(OX7B4DF339) as OX3B4C5D6E:
        for OX4B5C6D7E in OX3B4C5D6E:
            OX4B5C6D7E = OX4B5C6D7E.strip()
            if OX4B5C6D7E:
                OX1B2C3D4E, OX1A2B3C4D = OX4B5C6D7E.split(":")
                if OX1B2C3D4E == OX9A0B1C2D:
                    if OX8B9C0D1E[OX9F2A4881](OX1A2B3C4D, OX5E6F7G8H):
                        return True
    time.sleep(1 + random.random())
    return False