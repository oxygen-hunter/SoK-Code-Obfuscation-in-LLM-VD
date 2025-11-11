import base64
import functools
import hashlib
import os
import random
import time
from importlib import import_module

def load(cfg, log):
    t = cfg.get("auth", "type")
    log.debug("Authentication type is %s", t)
    if t == "None":
        c = NoneAuth
    elif t == "htpasswd":
        c = Auth
    else:
        c = import_module(t).Auth
    return c(cfg, log)

class BaseAuth:
    def __init__(self, cfg, log):
        self.logger, self.configuration = log, cfg

    def is_authenticated(self, u, p):
        raise NotImplementedError

    def map_login_to_user(self, l):
        return l

class NoneAuth(BaseAuth):
    def is_authenticated(self, u, p):
        return True

class Auth(BaseAuth):
    def __init__(self, cfg, log):
        super().__init__(cfg, log)
        self.encryption = cfg.get("auth", "htpasswd_encryption")
        fn = cfg.get("auth", "htpasswd_filename")
        self.filename = os.path.expanduser(fn)
        if self.encryption == "ssha":
            self.verify = self._ssha
        elif self.encryption == "sha1":
            self.verify = self._sha1
        elif self.encryption == "plain":
            self.verify = self._plain
        elif self.encryption == "md5":
            try:
                from passlib.hash import apr_md5_crypt as a
            except ImportError:
                raise RuntimeError("The htpasswd encryption method 'md5' requires the passlib module.")
            self.verify = functools.partial(self._md5apr1, a)
        elif self.encryption == "bcrypt":
            try:
                from passlib.hash import bcrypt as b
            except ImportError:
                raise RuntimeError("The htpasswd encryption method 'bcrypt' requires the passlib module with bcrypt support.")
            b.encrypt("test-bcrypt-backend")
            self.verify = functools.partial(self._bcrypt, b)
        elif self.encryption == "crypt":
            try:
                import crypt as c
            except ImportError:
                raise RuntimeError("The htpasswd encryption method 'crypt' requires the crypt() system support.")
            self.verify = functools.partial(self._crypt, c)
        else:
            raise RuntimeError("The htpasswd encryption method '%s' is not supported." % self.encryption)

    def _plain(self, hv, pw):
        return hv == pw

    def _crypt(self, c, hv, pw):
        return c.crypt(pw, hv) == hv

    def _sha1(self, hv, pw):
        v = hv.replace("{SHA}", "").encode("ascii")
        pw = pw.encode(self.configuration.get("encoding", "stock"))
        s = hashlib.sha1()
        s.update(pw)
        return s.digest() == base64.b64decode(v)

    def _ssha(self, hv, pw):
        v = hv.replace("{SSHA}", "").encode("ascii").decode("base64")
        pw = pw.encode(self.configuration.get("encoding", "stock"))
        h = v[:20]
        s = v[20:]
        sha1 = hashlib.sha1()
        sha1.update(pw)
        sha1.update(s)
        return sha1.digest() == h

    def _bcrypt(self, b, hv, pw):
        return b.verify(pw, hv)

    def _md5apr1(self, m, hv, pw):
        return m.verify(pw, hv)

    def is_authenticated(self, u, p):
        with open(self.filename) as f:
            for l in f:
                l = l.strip()
                if l:
                    lgn, hv = l.split(":")
                    if lgn == u and self.verify(hv, p):
                        return True
        time.sleep(1 + random.random())
        return False