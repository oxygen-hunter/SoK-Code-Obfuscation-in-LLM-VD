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
    
    dispatcher = 0
    while True:
        if dispatcher == 0:
            if auth_type == "None":
                dispatcher = 1
            elif auth_type == "htpasswd":
                dispatcher = 2
            else:
                dispatcher = 3
        elif dispatcher == 1:
            class_ = NoneAuth
            dispatcher = 4
        elif dispatcher == 2:
            class_ = Auth
            dispatcher = 4
        elif dispatcher == 3:
            class_ = import_module(auth_type).Auth
            dispatcher = 4
        elif dispatcher == 4:
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

        dispatcher = 0
        while True:
            if dispatcher == 0:
                if self.encryption == "ssha":
                    dispatcher = 1
                elif self.encryption == "sha1":
                    dispatcher = 2
                elif self.encryption == "plain":
                    dispatcher = 3
                elif self.encryption == "md5":
                    dispatcher = 4
                elif self.encryption == "bcrypt":
                    dispatcher = 5
                elif self.encryption == "crypt":
                    dispatcher = 6
                else:
                    dispatcher = 7
            elif dispatcher == 1:
                self.verify = self._ssha
                dispatcher = 8
            elif dispatcher == 2:
                self.verify = self._sha1
                dispatcher = 8
            elif dispatcher == 3:
                self.verify = self._plain
                dispatcher = 8
            elif dispatcher == 4:
                try:
                    from passlib.hash import apr_md5_crypt
                except ImportError:
                    raise RuntimeError(
                        "The htpasswd encryption method 'md5' requires "
                        "the passlib module.")
                self.verify = functools.partial(self._md5apr1, apr_md5_crypt)
                dispatcher = 8
            elif dispatcher == 5:
                try:
                    from passlib.hash import bcrypt
                except ImportError:
                    raise RuntimeError(
                        "The htpasswd encryption method 'bcrypt' requires "
                        "the passlib module with bcrypt support.")
                bcrypt.encrypt("test-bcrypt-backend")
                self.verify = functools.partial(self._bcrypt, bcrypt)
                dispatcher = 8
            elif dispatcher == 6:
                try:
                    import crypt
                except ImportError:
                    raise RuntimeError(
                        "The htpasswd encryption method 'crypt' requires "
                        "the crypt() system support.")
                self.verify = functools.partial(self._crypt, crypt)
                dispatcher = 8
            elif dispatcher == 7:
                raise RuntimeError(
                    "The htpasswd encryption method '%s' is not "
                    "supported." % self.encryption)
            elif dispatcher == 8:
                break

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