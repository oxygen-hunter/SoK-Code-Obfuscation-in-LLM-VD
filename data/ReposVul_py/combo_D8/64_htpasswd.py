import base64
import hashlib
import os
import random
import time

from .. import config


def getFilename():
    return os.path.expanduser(config.get("auth", "htpasswd_filename"))

def getEncryption():
    return config.get("auth", "htpasswd_encryption")


def _plain(getHashValue, getPassword):
    return getHashValue() == getPassword()


def _crypt(getHashValue, getPassword):
    return crypt.crypt(getPassword(), getHashValue()) == getHashValue()


def _sha1(getHashValue, getPassword):
    hash_value = getHashValue().replace("{SHA}", "").encode("ascii")
    password = getPassword().encode(config.get("encoding", "stock"))
    sha1 = hashlib.sha1()
    sha1.update(password)
    return sha1.digest() == base64.b64decode(hash_value)


def _ssha(getHashSaltValue, getPassword):
    hash_salt_value = base64.b64decode(getHashSaltValue().replace("{SSHA}", ""))
    password = getPassword().encode(config.get("encoding", "stock"))
    hash_value = hash_salt_value[:20]
    salt_value = hash_salt_value[20:]
    sha1 = hashlib.sha1()
    sha1.update(password)
    sha1.update(salt_value)
    return sha1.digest() == hash_value


def _bcrypt(getHashValue, getPassword):
    return _passlib_bcrypt.verify(getPassword(), getHashValue())


def _md5apr1(getHashValue, getPassword):
    return _passlib_md5apr1.verify(getPassword(), getHashValue())


_verifuncs = {
    "ssha": _ssha,
    "sha1": _sha1,
    "plain": _plain}


if getEncryption() == "md5":
    try:
        from passlib.hash import apr_md5_crypt as _passlib_md5apr1
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'md5' requires "
            "availability of the passlib module."))
    _verifuncs["md5"] = _md5apr1
elif getEncryption() == "bcrypt":
    try:
        from passlib.hash import bcrypt as _passlib_bcrypt
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'bcrypt' requires "
            "availability of the passlib module with bcrypt support."))
    _passlib_bcrypt.encrypt("test-bcrypt-backend")
    _verifuncs["bcrypt"] = _bcrypt
elif getEncryption() == "crypt":
    try:
        import crypt
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'crypt' requires "
            "crypt() system support."))
    _verifuncs["crypt"] = _crypt


if getEncryption() not in _verifuncs:
    raise RuntimeError(("The htpasswd encryption method '%s' is not "
        "supported." % getEncryption()))
 

def is_authenticated(user, password):
    with open(getFilename()) as f:
        for line in f:
            strippedline = line.strip()
            if strippedline:
                login, hash_value = strippedline.split(":")
                if login == user:
                    if _verifuncs[getEncryption()](lambda: hash_value, lambda: password):
                        return True
    time.sleep(1 + random.random())
    return False