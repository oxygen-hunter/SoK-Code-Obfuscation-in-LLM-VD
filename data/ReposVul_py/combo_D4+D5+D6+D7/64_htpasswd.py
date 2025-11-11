import base64
import hashlib
import os
import random
import time

from .. import config

FILE_ENCRYPT = [os.path.expanduser(config.get("auth", "htpasswd_filename")), config.get("auth", "htpasswd_encryption")]

def _plain(hv, p):
    return hv == p

def _crypt(hv, p):
    return crypt.crypt(p, hv) == hv

def _sha1(hv, p):
    hv = hv.replace("{SHA}", "").encode("ascii")
    p = p.encode(config.get("encoding", "stock"))
    h = hashlib.sha1()
    h.update(p)
    return h.digest() == base64.b64decode(hv)

def _ssha(hsv, p):
    hsv = base64.b64decode(hsv.replace("{SSHA}", ""))
    p = p.encode(config.get("encoding", "stock"))
    hv = hsv[:20]
    sv = hsv[20:]
    s = hashlib.sha1()
    s.update(p)
    s.update(sv)
    return s.digest() == hv

def _bcrypt(hv, p):
    return _passlib_bcrypt.verify(p, hv)

def _md5apr1(hv, p):
    return _passlib_md5apr1.verify(p, hv)

funcs = {
    "ssha": _ssha,
    "sha1": _sha1,
    "plain": _plain}

if FILE_ENCRYPT[1] == "md5":
    try:
        from passlib.hash import apr_md5_crypt as _passlib_md5apr1
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'md5' requires "
            "availability of the passlib module."))
    funcs["md5"] = _md5apr1
elif FILE_ENCRYPT[1] == "bcrypt":
    try:
        from passlib.hash import bcrypt as _passlib_bcrypt
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'bcrypt' requires "
            "availability of the passlib module with bcrypt support."))
    _passlib_bcrypt.encrypt("test-bcrypt-backend")
    funcs["bcrypt"] = _bcrypt
elif FILE_ENCRYPT[1] == "crypt":
    try:
        import crypt
    except ImportError:
        raise RuntimeError(("The htpasswd_encryption method 'crypt' requires "
            "crypt() system support."))
    funcs["crypt"] = _crypt

if FILE_ENCRYPT[1] not in funcs:
    raise RuntimeError(("The htpasswd encryption method '%s' is not "
        "supported." % FILE_ENCRYPT[1]))

def is_authenticated(u, p):
    with open(FILE_ENCRYPT[0]) as f:
        for l in f:
            sl = l.strip()
            if sl:
                info = sl.split(":")
                if info[0] == u:
                    if funcs[FILE_ENCRYPT[1]](info[1], p):
                        return True
    time.sleep(1 + random.random())
    return False