import abc
import json
import logging
import os
import pathlib
import stat
import typing
from typing import Optional

import httpx
import jwt

from . import http
from .constants import ENV_API_KEY, PLANET_BASE_URL, SECRET_FILE_PATH
from .exceptions import AuthException

LOGGER = logging.getLogger(__name__)

BASE_URL = f'{PLANET_BASE_URL}/v0/auth'

AuthType = httpx.Auth

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self):
        while self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            if opcode == "PUSH":
                self.stack.append(args[0])
            elif opcode == "POP":
                self.stack.pop()
            elif opcode == "ADD":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif opcode == "SUB":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif opcode == "JMP":
                self.pc = args[0] - 1
            elif opcode == "JZ":
                if self.stack.pop() == 0:
                    self.pc = args[0] - 1
            elif opcode == "LOAD":
                self.stack.append(self.stack[args[0]])
            elif opcode == "STORE":
                self.stack[args[0]] = self.stack.pop()
            elif opcode == "CALL_FUNC":
                func = args[0]
                nargs = args[1]
                params = [self.stack.pop() for _ in range(nargs)]
                result = func(*params)
                self.stack.append(result)
            self.pc += 1

    def load_instructions(self, instructions):
        self.instructions = instructions

def vm_instructions_for_auth_from_key(key):
    return [
        ("PUSH", key),
        ("CALL_FUNC", APIKeyAuth, 1),
    ]

def vm_instructions_for_auth_from_file(filename):
    return [
        ("PUSH", filename),
        ("CALL_FUNC", _SecretFile, 1),
        ("CALL_FUNC", lambda f: f.read(), 1),
        ("CALL_FUNC", APIKeyAuth.from_dict, 1),
    ]

def vm_instructions_for_auth_from_env(variable_name):
    return [
        ("PUSH", variable_name),
        ("CALL_FUNC", os.getenv, 1),
        ("CALL_FUNC", APIKeyAuth, 1),
    ]

def vm_instructions_for_auth_from_login(email, password, base_url):
    return [
        ("PUSH", base_url),
        ("CALL_FUNC", AuthClient, 1),
        ("PUSH", email),
        ("PUSH", password),
        ("CALL_FUNC", lambda cl, em, pw: cl.login(em, pw), 3),
        ("LOAD", 0),
        ("CALL_FUNC", APIKeyAuth, 1),
    ]

class Auth(metaclass=abc.ABCMeta):
    @staticmethod
    def from_key(key: str) -> AuthType:
        vm = VM()
        vm.load_instructions(vm_instructions_for_auth_from_key(key))
        vm.run()
        auth = vm.stack.pop()
        LOGGER.debug('Auth obtained from api key.')
        return auth

    @staticmethod
    def from_file(filename: Optional[typing.Union[str, pathlib.Path]] = None) -> AuthType:
        filename = filename or SECRET_FILE_PATH
        vm = VM()
        vm.load_instructions(vm_instructions_for_auth_from_file(filename))
        vm.run()
        auth = vm.stack.pop()
        LOGGER.debug(f'Auth read from secret file {filename}.')
        return auth

    @staticmethod
    def from_env(variable_name: Optional[str] = None) -> AuthType:
        variable_name = variable_name or ENV_API_KEY
        vm = VM()
        vm.load_instructions(vm_instructions_for_auth_from_env(variable_name))
        vm.run()
        auth = vm.stack.pop()
        LOGGER.debug(f'Auth set from environment variable {variable_name}')
        return auth

    @staticmethod
    def from_login(email: str, password: str, base_url: Optional[str] = None) -> AuthType:
        vm = VM()
        vm.load_instructions(vm_instructions_for_auth_from_login(email, password, base_url))
        vm.run()
        auth = vm.stack.pop()
        LOGGER.debug('Auth set from login email and password')
        return auth

    @classmethod
    @abc.abstractmethod
    def from_dict(cls, data: dict) -> AuthType:
        pass

    @property
    @abc.abstractmethod
    def value(self):
        pass

    @abc.abstractmethod
    def to_dict(self) -> dict:
        pass

    def store(self, filename: Optional[typing.Union[str, pathlib.Path]] = None):
        filename = filename or SECRET_FILE_PATH
        secret_file = _SecretFile(filename)
        secret_file.write(self.to_dict())

class AuthClient:
    def __init__(self, base_url: Optional[str] = None):
        self._base_url = base_url or BASE_URL
        if self._base_url.endswith('/'):
            self._base_url = self._base_url[:-1]

    def login(self, email: str, password: str) -> dict:
        url = f'{self._base_url}/login'
        data = {'email': email, 'password': password}
        sess = http.AuthSession()
        resp = sess.request(url=url, method='POST', json=data)
        return self.decode_response(resp)

    @staticmethod
    def decode_response(response):
        token = response.json()['token']
        return jwt.decode(token, options={'verify_signature': False})

class APIKeyAuthException(Exception):
    pass

class APIKeyAuth(httpx.BasicAuth, Auth):
    DICT_KEY = 'key'

    def __init__(self, key: str):
        if not key:
            raise APIKeyAuthException('API key cannot be empty.')
        self._key = key
        super().__init__(self._key, '')

    @classmethod
    def from_dict(cls, data: dict) -> APIKeyAuth:
        api_key = data[cls.DICT_KEY]
        return cls(api_key)

    def to_dict(self):
        return {self.DICT_KEY: self._key}

    @property
    def value(self):
        return self._key

class _SecretFile:
    def __init__(self, path: typing.Union[str, pathlib.Path]):
        self.path = pathlib.Path(path)
        self.permissions = stat.S_IRUSR | stat.S_IWUSR
        self._enforce_permissions()

    def write(self, contents: dict):
        try:
            secrets_to_write = self.read()
            secrets_to_write.update(contents)
        except (FileNotFoundError, KeyError, json.decoder.JSONDecodeError):
            secrets_to_write = contents
        self._write(secrets_to_write)

    def _write(self, contents: dict):
        LOGGER.debug(f'Writing to {self.path}')
        def opener(path, flags):
            return os.open(path, flags, self.permissions)
        with open(self.path, 'w', opener=opener) as fp:
            fp.write(json.dumps(contents))

    def read(self) -> dict:
        LOGGER.debug(f'Reading from {self.path}')
        with open(self.path, 'r') as fp:
            contents = json.loads(fp.read())
        return contents

    def _enforce_permissions(self):
        try:
            file_permissions = self.path.stat().st_mode & 0o777
            if file_permissions != self.permissions:
                LOGGER.debug(
                    f'{self.path} permissions are {oct(file_permissions)}, '
                    f'should be {oct(self.permissions)}. Fixing.')
                self.path.chmod(self.permissions)
        except FileNotFoundError:
            pass