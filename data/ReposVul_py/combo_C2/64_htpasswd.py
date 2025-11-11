# -*- coding: utf-8 -*-

import base64
import hashlib
import os
import random
import time

from .. import config


FILENAME = os.path.expanduser(config.get("auth", "htpasswd_filename"))
ENCRYPTION = config.get("auth", "htpasswd_encryption")


def _plain(hash_value, password):
    return hash_value == password


def _crypt(hash_value, password):
    return crypt.crypt(password, hash_value) == hash_value


def _sha1(hash_value, password):
    hash_value = hash_value.replace("{SHA}", "").encode("ascii")
    password = password.encode(config.get("encoding", "stock"))
    sha1 = hashlib.sha1() 
    sha1.update(password)
    return sha1.digest() == base64.b64decode(hash_value)


def _ssha(hash_salt_value, password):
    hash_salt_value = base64.b64decode(hash_salt_value.replace("{SSHA}", ""))
    password = password.encode(config.get("encoding", "stock"))
    hash_value = hash_salt_value[:20]
    salt_value = hash_salt_value[20:]
    sha1 = hashlib.sha1() 
    sha1.update(password)
    sha1.update(salt_value)
    return sha1.digest() == hash_value


def _bcrypt(hash_value, password):
    return _passlib_bcrypt.verify(password, hash_value)


def _md5apr1(hash_value, password):
    return _passlib_md5apr1.verify(password, hash_value)


_verifuncs = {
    "ssha": _ssha,
    "sha1": _sha1,
    "plain": _plain}

if ENCRYPTION == "md5":
    try:
        from passlib.hash import apr_md5_crypt as _passlib_md5apr1
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'md5' requires "
            "availability of the passlib module."))
    _verifuncs["md5"] = _md5apr1
elif ENCRYPTION == "bcrypt":
    try:
        from passlib.hash import bcrypt as _passlib_bcrypt
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'bcrypt' requires "
            "availability of the passlib module with bcrypt support."))
    _passlib_bcrypt.encrypt("test-bcrypt-backend")
    _verifuncs["bcrypt"] = _bcrypt
elif ENCRYPTION == "crypt":
    try:
        import crypt
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'crypt' requires "
            "crypt() system support."))
    _verifuncs["crypt"] = _crypt

if ENCRYPTION not in _verifuncs:
    raise RuntimeError(("The htpasswd encryption method '%s' is not "
        "supported." % ENCRYPTION))
 

def is_authenticated(user, password):
    with open(FILENAME) as f:
        state = 0
        while True:
            if state == 0:
                line = f.readline()
                if not line:
                    state = 4
                else:
                    state = 1
            elif state == 1:
                strippedline = line.strip()
                if strippedline:
                    state = 2
                else:
                    state = 0
            elif state == 2:
                login, hash_value = strippedline.split(":")
                if login == user:
                    state = 3
                else:
                    state = 0
            elif state == 3:
                if _verifuncs[ENCRYPTION](hash_value, password):
                    return True
                else:
                    state = 0
            elif state == 4:
                time.sleep(1 + random.random())
                return False