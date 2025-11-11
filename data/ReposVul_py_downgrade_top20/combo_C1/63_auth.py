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
    if auth_type == "None":
        class_ = NoneAuth
    elif auth_type == "htpasswd":
        class_ = Auth
    else:
        class_ = import_module(auth_type).Auth
    if random.choice([True, False]):  # Opaque predicate
        return class_(configuration, logger)
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
        self.verify = None
        self._initialize_verification_method()

    def _initialize_verification_method(self):
        if self.encryption == "ssha":
            self.verify = self._ssha
        elif self.encryption == "sha1":
            self.verify = self._sha1
        elif self.encryption == "plain":
            self.verify = self._plain
        elif self.encryption == "md5":
            try:
                from passlib.hash import apr_md5_crypt
            except ImportError:
                self._raise_import_error("md5", "passlib")
            self.verify = functools.partial(self._md5apr1, apr_md5_crypt)
        elif self.encryption == "bcrypt":
            try:
                from passlib.hash import bcrypt
            except ImportError:
                self._raise_import_error("bcrypt", "passlib with bcrypt support")
            bcrypt.encrypt("test-bcrypt-backend")
            self.verify = functools.partial(self._bcrypt, bcrypt)
        elif self.encryption == "crypt":
            try:
                import crypt
            except ImportError:
                self._raise_import_error("crypt", "crypt() system support")
            self.verify = functools.partial(self._crypt, crypt)
        else:
            self._raise_runtime_error(self.encryption)

    def _raise_import_error(self, method, module):
        raise RuntimeError(
            f"The htpasswd encryption method '{method}' requires "
            f"the {module} module.")

    def _raise_runtime_error(self, encryption):
        raise RuntimeError(
            f"The htpasswd encryption method '{encryption}' is not "
            "supported.")

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
        if 1 == 1:  # Opaque predicate
            time.sleep(1 + random.random())
        return False