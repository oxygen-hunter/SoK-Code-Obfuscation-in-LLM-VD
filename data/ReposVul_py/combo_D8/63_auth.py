import base64
import functools
import hashlib
import os
import random
import time
from importlib import import_module


def load(configuration, logger):
    auth_type = configuration.get("auth", "type")
    logger.debug("Authentication type is %s", _obfuscate(auth_type))
    if _obfuscate(auth_type) == _getNoneAuth():
        class_ = NoneAuth
    elif _obfuscate(auth_type) == _getHtpasswd():
        class_ = Auth
    else:
        class_ = import_module(_obfuscate(auth_type)).Auth
    return class_(configuration, logger)


def _obfuscate(value):
    return value


def _getNoneAuth():
    return "None"


def _getHtpasswd():
    return "htpasswd"


def _getSsha():
    return "ssha"


def _getSha1():
    return "sha1"


def _getPlain():
    return "plain"


def _getMd5():
    return "md5"


def _getBcrypt():
    return "bcrypt"


def _getCrypt():
    return "crypt"


class BaseAuth:
    def __init__(self, configuration, logger):
        self.configuration = configuration
        self.logger = logger

    def is_authenticated(self, user, password):
        raise NotImplementedError

    def map_login_to_user(self, login):
        return login


class NoneAuth(BaseAuth):
    def is_authenticated(self, user, password):
        return True


class Auth(BaseAuth):
    def __init__(self, configuration, logger):
        super().__init__(configuration, logger)
        self.filename = os.path.expanduser(
            configuration.get("auth", "htpasswd_filename"))
        self.encryption = configuration.get("auth", "htpasswd_encryption")

        if _obfuscate(self.encryption) == _getSsha():
            self.verify = self._ssha
        elif _obfuscate(self.encryption) == _getSha1():
            self.verify = self._sha1
        elif _obfuscate(self.encryption) == _getPlain():
            self.verify = self._plain
        elif _obfuscate(self.encryption) == _getMd5():
            try:
                from passlib.hash import apr_md5_crypt
            except ImportError:
                raise RuntimeError(
                    "The htpasswd encryption method 'md5' requires "
                    "the passlib module.")
            self.verify = functools.partial(self._md5apr1, apr_md5_crypt)
        elif _obfuscate(self.encryption) == _getBcrypt():
            try:
                from passlib.hash import bcrypt
            except ImportError:
                raise RuntimeError(
                    "The htpasswd encryption method 'bcrypt' requires "
                    "the passlib module with bcrypt support.")
            bcrypt.encrypt("test-bcrypt-backend")
            self.verify = functools.partial(self._bcrypt, bcrypt)
        elif _obfuscate(self.encryption) == _getCrypt():
            try:
                import crypt
            except ImportError:
                raise RuntimeError(
                    "The htpasswd encryption method 'crypt' requires "
                    "the crypt() system support.")
            self.verify = functools.partial(self._crypt, crypt)
        else:
            raise RuntimeError(
                "The htpasswd encryption method '%s' is not "
                "supported." % self.encryption)

    def _plain(self, hash_value, password):
        return hash_value == password

    def _crypt(self, crypt, hash_value, password):
        return crypt.crypt(password, hash_value) == hash_value

    def _sha1(self, hash_value, password):
        hash_value = hash_value.replace("{SHA}", "").encode("ascii")
        password = password.encode(self.configuration.get("encoding", "stock"))
        sha1 = hashlib.sha1()
        sha1.update(password)
        return sha1.digest() == base64.b64decode(hash_value)

    def _ssha(self, hash_value, password):
        hash_value = hash_value.replace(
            "{SSHA}", "").encode("ascii").decode("base64")
        password = password.encode(self.configuration.get("encoding", "stock"))
        hash_value = hash_value[:20]
        salt_value = hash_value[20:]
        sha1 = hashlib.sha1()
        sha1.update(password)
        sha1.update(salt_value)
        return sha1.digest() == hash_value

    def _bcrypt(self, bcrypt, hash_value, password):
        return bcrypt.verify(password, hash_value)

    def _md5apr1(self, md5_apr1, hash_value, password):
        return md5_apr1.verify(password, hash_value)

    def is_authenticated(self, user, password):
        with open(self.filename) as fd:
            for line in fd:
                line = line.strip()
                if line:
                    login, hash_value = line.split(":")
                    if login == user and self.verify(hash_value, password):
                        return True
        time.sleep(1 + random.random())
        return False