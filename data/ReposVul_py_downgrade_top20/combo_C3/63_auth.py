import base64
import functools
import hashlib
import os
import random
import time
from importlib import import_module

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.labels = {}

    def load_instructions(self, instructions):
        self.instructions = instructions
        self._resolve_labels()

    def _resolve_labels(self):
        for index, instr in enumerate(self.instructions):
            if isinstance(instr, tuple) and instr[0] == "LABEL":
                self.labels[instr[1]] = index

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            if isinstance(instr, str):
                getattr(self, f"op_{instr}")()
            elif isinstance(instr, tuple):
                getattr(self, f"op_{instr[0]}")(*instr[1:])
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_LOAD(self, index):
        self.stack.append(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.stack.pop()

    def op_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, label):
        self.pc = self.labels[label] - 1

    def op_JZ(self, label):
        if self.stack.pop() == 0:
            self.pc = self.labels[label] - 1

def load_vm(configuration, logger):
    vm = VM()
    auth_type = configuration.get("auth", "type")
    logger.debug("Authentication type is %s", auth_type)
    if auth_type == "None":
        vm.load_instructions([
            ("PUSH", NoneAuth(configuration, logger)),
            "POP"
        ])
    elif auth_type == "htpasswd":
        vm.load_instructions([
            ("PUSH", Auth(configuration, logger)),
            "POP"
        ])
    else:
        vm.load_instructions([
            ("PUSH", import_module(auth_type).Auth),
            "POP"
        ])
    vm.run()
    return vm.op_POP()

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
                raise RuntimeError("The htpasswd encryption method 'md5' requires the passlib module.")
            self.verify = functools.partial(self._md5apr1, apr_md5_crypt)
        elif self.encryption == "bcrypt":
            try:
                from passlib.hash import bcrypt
            except ImportError:
                raise RuntimeError("The htpasswd encryption method 'bcrypt' requires the passlib module with bcrypt support.")
            bcrypt.encrypt("test-bcrypt-backend")
            self.verify = functools.partial(self._bcrypt, bcrypt)
        elif self.encryption == "crypt":
            try:
                import crypt
            except ImportError:
                raise RuntimeError("The htpasswd encryption method 'crypt' requires the crypt() system support.")
            self.verify = functools.partial(self._crypt, crypt)
        else:
            raise RuntimeError("The htpasswd encryption method '%s' is not supported." % self.encryption)

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
        hash_value = hash_value.replace("{SSHA}", "").encode("ascii").decode("base64")
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