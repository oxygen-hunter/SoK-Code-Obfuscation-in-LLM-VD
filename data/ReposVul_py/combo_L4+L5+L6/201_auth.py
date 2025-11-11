from __future__ import annotations
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


class Auth(metaclass=abc.ABCMeta):

    @staticmethod
    def from_key(key: str) -> AuthType:
        auth = APIKeyAuth(key=key)
        LOGGER.debug('Auth obtained from api key.')
        return auth

    @staticmethod
    def from_file(
        filename: Optional[typing.Union[str,
                                        pathlib.Path]] = None) -> AuthType:
        filename = filename or SECRET_FILE_PATH

        def handle_exception(e):
            if isinstance(e, FileNotFoundError):
                raise AuthException(f'File {filename} does not exist.')
            elif isinstance(e, (KeyError, json.decoder.JSONDecodeError)):
                raise AuthException(f'File {filename} is not the correct format.')

        try:
            secrets = _SecretFile(filename).read()
            auth = APIKeyAuth.from_dict(secrets)
        except Exception as e:
            handle_exception(e)

        LOGGER.debug(f'Auth read from secret file {filename}.')
        return auth

    @staticmethod
    def from_env(variable_name: Optional[str] = None) -> AuthType:
        variable_name = variable_name or ENV_API_KEY
        api_key = os.getenv(variable_name, '')

        def handle_exception(e):
            if isinstance(e, APIKeyAuthException):
                raise AuthException(
                    f'Environment variable {variable_name} either does not exist '
                    'or is empty.')

        try:
            auth = APIKeyAuth(api_key)
            LOGGER.debug(f'Auth set from environment variable {variable_name}')
        except Exception as e:
            handle_exception(e)

        return auth

    @staticmethod
    def from_login(email: str,
                   password: str,
                   base_url: Optional[str] = None) -> AuthType:
        cl = AuthClient(base_url=base_url)
        auth_data = cl.login(email, password)

        api_key = auth_data['api_key']
        auth = APIKeyAuth(api_key)
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

    def store(self,
              filename: Optional[typing.Union[str, pathlib.Path]] = None):
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
        def handle_exceptions(e):
            if isinstance(e, (FileNotFoundError, KeyError, json.decoder.JSONDecodeError)):
                return contents
        try:
            secrets_to_write = self.read()
            secrets_to_write.update(contents)
        except Exception as e:
            secrets_to_write = handle_exceptions(e)

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