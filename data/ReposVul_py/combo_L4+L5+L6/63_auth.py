import base64
import functools
import hashlib
import os
import random
import time
from importlib import import_module

def load(configuration, logger):
    auth_type = configuration.get("auth", "type")
    logger.debug("Authentication type is %s", auth_type)
    class_ = {
        "None": NoneAuth,
        "htpasswd": Auth
    }.get(auth_type, import_module(auth_type).Auth)
    return class_(configuration, logger)

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
        self.verify = self._select_verification_method()

    def _select_verification_method(self):
        if self.encryption == "ssha":
            return self._ssha
        if self.encryption == "sha1":
            return self._sha1
        if self.encryption == "plain":
            return self._plain
        if self.encryption == "md5":
            try:
                from passlib.hash import apr_md5_crypt
            except ImportError:
                raise RuntimeError(
                    "The htpasswd encryption method 'md5' requires "
                    "the passlib module.")
            return functools.partial(self._md5apr1, apr_md5_crypt)
        if self.encryption == "bcrypt":
            try:
                from passlib.hash import bcrypt
            except ImportError:
                raise RuntimeError(
                    "The htpasswd encryption method 'bcrypt' requires "
                    "the passlib module with bcrypt support.")
            bcrypt.encrypt("test-bcrypt-backend")
            return functools.partial(self._bcrypt, bcrypt)
        if self.encryption == "crypt":
            try:
                import crypt
            except ImportError:
                raise RuntimeError(
                    "The htpasswd encryption method 'crypt' requires "
                    "the crypt() system support.")
            return functools.partial(self._crypt, crypt)
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
        return self._check_credentials(user, password, iter(open(self.filename)))

    def _check_credentials(self, user, password, file_iter):
        try:
            line = next(file_iter).strip()
            if line:
                login, hash_value = line.split(":")
                if login == user and self.verify(hash_value, password):
                    return True
            return self._check_credentials(user, password, file_iter)
        except StopIteration:
            time.sleep(1 + random.random())
            return False