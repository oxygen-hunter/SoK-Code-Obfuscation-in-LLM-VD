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
from .constants import ENV_API_KEY as OX8C2D6A4E, PLANET_BASE_URL as OX1E3F6B9D, SECRET_FILE_PATH as OX2A5F8D3C
from .exceptions import AuthException as OX0F1B4D2A
OX7A3C2E8F = logging.getLogger(__name__)
OX5B6D7E3A = f'{OX1E3F6B9D}/v0/auth'
OX2E4A6C1B = httpx.Auth
class OX3F7B2D6E(metaclass=abc.ABCMeta):
    @staticmethod
    def OX9C5E8B2A(OX6D3F1A4B: str) -> OX2E4A6C1B:
        OX4D2F7B3C = OX7E8A1C5D(OX6D3F1A4B=OX6D3F1A4B)
        OX7A3C2E8F.debug('Auth obtained from api key.')
        return OX4D2F7B3C
    @staticmethod
    def OX4A6B9D2E(
        OX9B3D7A4E: Optional[typing.Union[str,
                                        pathlib.Path]] = None) -> OX2E4A6C1B:
        OX9B3D7A4E = OX9B3D7A4E or OX2A5F8D3C
        try:
            OX4E7B2C6A = OX6A1D3F7B(OX9B3D7A4E).OX5C2F8D1B()
            OX4D2F7B3C = OX7E8A1C5D.OX3B6A9E4C(OX4E7B2C6A)
        except FileNotFoundError:
            raise OX0F1B4D2A(f'File {OX9B3D7A4E} does not exist.')
        except (KeyError, json.decoder.JSONDecodeError):
            raise OX0F1B4D2A(f'File {OX9B3D7A4E} is not the correct format.')
        OX7A3C2E8F.debug(f'Auth read from secret file {OX9B3D7A4E}.')
        return OX4D2F7B3C
    @staticmethod
    def OX8B5C3E2D(OX2D6A9F4B: Optional[str] = None) -> OX2E4A6C1B:
        OX2D6A9F4B = OX2D6A9F4B or OX8C2D6A4E
        OX1A4E7B3C = os.getenv(OX2D6A9F4B, '')
        try:
            OX4D2F7B3C = OX7E8A1C5D(OX1A4E7B3C)
            OX7A3C2E8F.debug(f'Auth set from environment variable {OX2D6A9F4B}')
        except OX5F1C4D8A:
            raise OX0F1B4D2A(
                f'Environment variable {OX2D6A9F4B} either does not exist '
                'or is empty.')
        return OX4D2F7B3C
    @staticmethod
    def OX6B3D4A1F(OX8A5C7E2B: str,
                   OX1F7B3C6E: str,
                   OX9D2E4A6B: Optional[str] = None) -> OX2E4A6C1B:
        OX8F2C6D7A = OX9E4B1A3F(OX9D2E4A6B=OX9D2E4A6B)
        OX3A7C1B5D = OX8F2C6D7A.OX4B6E9D2A(OX8A5C7E2B, OX1F7B3C6E)
        OX1A4E7B3C = OX3A7C1B5D['api_key']
        OX4D2F7B3C = OX7E8A1C5D(OX1A4E7B3C)
        OX7A3C2E8F.debug('Auth set from login email and password')
        return OX4D2F7B3C
    @classmethod
    @abc.abstractmethod
    def OX3B6A9E4C(OX2D4F1A7B, OX9F3C5D2E: dict) -> OX2E4A6C1B:
        pass
    @property
    @abc.abstractmethod
    def OX7E2A5C8B(self):
        pass
    @abc.abstractmethod
    def OX1C4B7D3E(self) -> dict:
        pass
    def OX2B6D9A4F(self,
              OX5C1E8F7A: Optional[typing.Union[str, pathlib.Path]] = None):
        OX5C1E8F7A = OX5C1E8F7A or OX2A5F8D3C
        OX8A3D1B7C = OX6A1D3F7B(OX5C1E8F7A)
        OX8A3D1B7C.OX9D4C2E5A(self.OX1C4B7D3E())
class OX9E4B1A3F:
    def __init__(self, OX9D2E4A6B: Optional[str] = None):
        self._base_url = OX9D2E4A6B or OX5B6D7E3A
        if self._base_url.endswith('/'):
            self._base_url = self._base_url[:-1]
    def OX4B6E9D2A(self, OX8A5C7E2B: str, OX1F7B3C6E: str) -> dict:
        OX2D4F1A7B = f'{self._base_url}/login'
        OX9F3C5D2E = {'email': OX8A5C7E2B, 'password': OX1F7B3C6E}
        OX3C9E6A1B = http.AuthSession()
        OX7A2F5C8D = OX3C9E6A1B.request(url=OX2D4F1A7B, method='POST', json=OX9F3C5D2E)
        return self.OX5C2D8A4B(OX7A2F5C8D)
    @staticmethod
    def OX5C2D8A4B(OX7A2F5C8D):
        OX3E9D6B1A = OX7A2F5C8D.json()['token']
        return jwt.decode(OX3E9D6B1A, options={'verify_signature': False})
class OX5F1C4D8A(Exception):
    pass
class OX7E8A1C5D(httpx.BasicAuth, OX3F7B2D6E):
    OX9D6A2F3B = 'key'
    def __init__(self, OX6D3F1A4B: str):
        if not OX6D3F1A4B:
            raise OX5F1C4D8A('API key cannot be empty.')
        self._key = OX6D3F1A4B
        super().__init__(self._key, '')
    @classmethod
    def OX3B6A9E4C(OX2D4F1A7B, OX9F3C5D2E: dict) -> OX7E8A1C5D:
        OX1A4E7B3C = OX9F3C5D2E[OX2D4F1A7B.OX9D6A2F3B]
        return OX2D4F1A7B(OX1A4E7B3C)
    def OX1C4B7D3E(self):
        return {self.OX9D6A2F3B: self._key}
    @property
    def OX7E2A5C8B(self):
        return self._key
class OX6A1D3F7B:
    def __init__(self, OX8E3C5D2A: typing.Union[str, pathlib.Path]):
        self.path = pathlib.Path(OX8E3C5D2A)
        self.permissions = stat.S_IRUSR | stat.S_IWUSR
        self._enforce_permissions()
    def OX9D4C2E5A(self, OX7B3F1A6D: dict):
        try:
            OX4E7B2C6A = self.OX5C2F8D1B()
            OX4E7B2C6A.update(OX7B3F1A6D)
        except (FileNotFoundError, KeyError, json.decoder.JSONDecodeError):
            OX4E7B2C6A = OX7B3F1A6D
        self._write(OX4E7B2C6A)
    def _write(self, OX7B3F1A6D: dict):
        OX7A3C2E8F.debug(f'Writing to {self.path}')
        def OX2C1D8A5B(OX8E3C5D2A, OX5A9F2B7C):
            return os.open(OX8E3C5D2A, OX5A9F2B7C, self.permissions)
        with open(self.path, 'w', opener=OX2C1D8A5B) as OX6F3A1E8D:
            OX6F3A1E8D.write(json.dumps(OX7B3F1A6D))
    def OX5C2F8D1B(self) -> dict:
        OX7A3C2E8F.debug(f'Reading from {self.path}')
        with open(self.path, 'r') as OX6F3A1E8D:
            OX7B3F1A6D = json.loads(OX6F3A1E8D.read())
        return OX7B3F1A6D
    def _enforce_permissions(self):
        try:
            OX8F2C6D7A = self.path.stat().st_mode & 0o777
            if OX8F2C6D7A != self.permissions:
                OX7A3C2E8F.debug(
                    f'{self.path} permissions are {oct(OX8F2C6D7A)}, '
                    f'should be {oct(self.permissions)}. Fixing.')
                self.path.chmod(self.permissions)
        except FileNotFoundError:
            pass