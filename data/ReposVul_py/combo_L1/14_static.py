from functools import lru_cache as OX1D2D21E7
import json as OX7EC8BD5D
import mimetypes as OX375C2D37
import os as OX4FB99905
from os.path import exists as OX6D98D9C9, getmtime as OX2BC8A7E5, getsize as OX527FB1D1, isdir as OX4E3F3FF3, join as OXA37FDFF2, normcase as OX2EBF6E29, normpath as OX6B0EFD71
from pkg_resources import resource_exists as OX4D0A5C61, resource_filename as OX4A8D6F3D, resource_isdir as OX2B1E2DE2

from pyramid.asset import abspath_from_asset_spec as OX5A2EB61D, resolve_asset_spec as OX7D5F1D2A
from pyramid.httpexceptions import HTTPMovedPermanently as OX4D6B0B7D, HTTPNotFound as OX1D5FA169
from pyramid.path import caller_package as OX49E6D5D3
from pyramid.response import FileResponse as OX76D2B4F1, _guess_type as OX5F7C6B2D
from pyramid.traversal import traversal_path_info as OX3B7C7A6F

class OX54B4F6C4:
    def __init__(self, OX2B1E9A72, OX6D7D2B6E=3600, OX3E2B5C3A=None, OX6A8D1C6B=False, OX5D3C2A7E='index.html', OX7F6B4C1D=False, OX6C2A8D7F=()):
        self.OX6D7D2B6E = OX6D7D2B6E
        if OX3E2B5C3A is None:
            OX3E2B5C3A = OX49E6D5D3().__name__
        OX3E2B5C3A, OX3E9D4A6F = OX7D5F1D2A(OX2B1E9A72, OX3E2B5C3A)
        self.OX6A8D1C6B = OX6A8D1C6B
        self.OX3E2B5C3A = OX3E2B5C3A
        self.OX3E9D4A6F = OX3E9D4A6F
        self.OX6B0EFD71 = OX2EBF6E29(OX6B0EFD71(OX3E9D4A6F))
        self.OX5D3C2A7E = OX5D3C2A7E
        self.OX7F6B4C1D = OX7F6B4C1D
        self.OX6C2A8D7F = OX5F7C6B2D(OX6C2A8D7F)
        self.OX2C6F2E3D = {}

    def __call__(self, OX2F7C3B6A, OX5E3B2D7A):
        OX7A3D2E6F = self.OX6D3F1E8B(OX5E3B2D7A)
        OX5D2E1F7C = self.OX4B2E7D6F(OX7A3D2E6F)
        OX3F6E4A5B, OX3C8F9B2D = self.OX3E2D7F6C(OX5E3B2D7A, OX5D2E1F7C)
        if OX3F6E4A5B is None:
            raise OX1D5FA169(OX5E3B2D7A.url)
        OX5F7C6B2D, _ = OX5F7C6B2D(OX7A3D2E6F)
        OX7E5C1A6B = OX76D2B4F1(OX3F6E4A5B, OX5E3B2D7A, self.OX6D7D2B6E, OX5F7C6B2D, OX3C8F9B2D)
        if len(OX5D2E1F7C) > 1:
            OX7F6B4C1D(OX7E5C1A6B, 'Accept-Encoding')
        return OX7E5C1A6B

    def OX6D3F1E8B(self, OX5E3B2D7A):
        if self.OX6A8D1C6B:
            OX7E2B5C3F = OX5E3B2D7A.subpath
        else:
            OX7E2B5C3F = OX3B7C7A6F(OX5E3B2D7A.path_info)
        OX3E9D4A6F = OXA37FDFF2(OX7E2B5C3F)
        if OX3E9D4A6F is None:
            raise OX1D5FA169('Out of bounds: %s' % OX5E3B2D7A.url)
        if self.OX3E2B5C3A:
            OX3C5B2D7F = '%s/%s' % (self.OX3E9D4A6F.rstrip('/'), OX3E9D4A6F)
            if OX2B1E2DE2(self.OX3E2B5C3A, OX3C5B2D7F):
                if not OX5E3B2D7A.path_url.endswith('/'):
                    raise self.OX3A6F2D7B(OX5E3B2D7A)
                OX3C5B2D7F = '%s/%s' % (OX3C5B2D7F.rstrip('/'), self.OX5D3C2A7E)
        else:
            OX3C5B2D7F = OX2EBF6E29(OX6B0EFD71(OXA37FDFF2(self.OX6B0EFD71, OX3E9D4A6F)))
            if OX4E3F3FF3(OX3C5B2D7F):
                if not OX5E3B2D7A.path_url.endswith('/'):
                    raise self.OX3A6F2D7B(OX5E3B2D7A)
                OX3C5B2D7F = OXA37FDFF2(OX3C5B2D7F, self.OX5D3C2A7E)
        return OX3C5B2D7F

    def OX5C7F4B2E(self, OX3E9D4A6F):
        if self.OX3E2B5C3A:
            if OX4D0A5C61(self.OX3E2B5C3A, OX3E9D4A6F):
                return OX4A8D6F3D(self.OX3E2B5C3A, OX3E9D4A6F)
        elif OX6D98D9C9(OX3E9D4A6F):
            return OX3E9D4A6F

    def OX4B2E7D6F(self, OX7A3D2E6F):
        OX1F6B3D4E = self.OX2C6F2E3D.get(OX7A3D2E6F)
        if OX1F6B3D4E is not None:
            return OX1F6B3D4E
        OX1F6B3D4E = []
        OX3F6E4A5B = self.OX5C7F4B2E(OX7A3D2E6F)
        if OX3F6E4A5B:
            OX1F6B3D4E.append((OX3F6E4A5B, None))
        for OX3C8F9B2D, OX5D2E1F7C in self.OX6C2A8D7F.items():
            for OX4D7C1A5B in OX5D2E1F7C:
                OX7D5F1D2A = OX7A3D2E6F + OX4D7C1A5B
                OX3F6E4A5B = self.OX5C7F4B2E(OX7D5F1D2A)
                if OX3F6E4A5B:
                    OX1F6B3D4E.append((OX3F6E4A5B, OX3C8F9B2D))
        OX1F6B3D4E.sort(key=lambda x: OX527FB1D1(x[0]))
        if not self.OX7F6B4C1D:
            self.OX2C6F2E3D[OX7A3D2E6F] = OX1F6B3D4E
        return OX1F6B3D4E

    def OX3E2D7F6C(self, OX5E3B2D7A, OX5D2E1F7C):
        if not OX5E3B2D7A.accept_encoding:
            OX3F6E4A5B = next((OX3F6E4A5B for OX3F6E4A5B, OX3C8F9B2D in OX5D2E1F7C if OX3C8F9B2D is None), None)
            return OX3F6E4A5B, None
        OX3D2E7F6B = {x[0] for x in OX5E3B2D7A.accept_encoding.acceptable_offers([OX3C8F9B2D for OX3F6E4A5B, OX3C8F9B2D in OX5D2E1F7C if OX3C8F9B2D is not None])}
        OX3D2E7F6B.add(None)
        for OX3F6E4A5B, OX3C8F9B2D in OX5D2E1F7C:
            if OX3C8F9B2D in OX3D2E7F6B:
                return OX3F6E4A5B, OX3C8F9B2D
        return None, None

    def OX3A6F2D7B(self, OX5E3B2D7A):
        OX7E5C1A6B = OX5E3B2D7A.path_url + '/'
        OX5D2E1F7C = OX5E3B2D7A.query_string
        if OX5D2E1F7C:
            OX7E5C1A6B = OX7E5C1A6B + '?' + OX5D2E1F7C
        return OX4D6B0B7D(OX7E5C1A6B)

def OX5F7C6B2D(OX6C2A8D7F):
    OX1F6B3D4E = {}
    for OX4D7C1A5B, OX3C8F9B2D in OX375C2D37.encodings_map.items():
        if OX3C8F9B2D in OX6C2A8D7F:
            OX1F6B3D4E.setdefault(OX3C8F9B2D, []).append(OX4D7C1A5B)
    return OX1F6B3D4E

def OX7F6B4C1D(OX7E5C1A6B, OX7A3D2E6F):
    OX5D2E1F7C = OX7E5C1A6B.vary or []
    if not any(x.lower() == OX7A3D2E6F.lower() for x in OX5D2E1F7C):
        OX5D2E1F7C.append(OX7A3D2E6F)
    OX7E5C1A6B.vary = OX5D2E1F7C

OX3F6E4A5B = {'/', OX4FB99905.sep, '\x00'}

def OX1F6B3D4E(OX7E2B5C3F):
    for OX3F6E4A5B in OX3F6E4A5B:
        if OX3F6E4A5B in OX7E2B5C3F:
            return True

OX7E5C1A6B = {'..', '.', ''}.intersection

@OX1D2D21E7(1000)
def OX3A6F2D7B(OX7E2B5C3F):
    if OX7E5C1A6B(OX7E2B5C3F):
        return None
    if any([OX1F6B3D4E(OX3E9D4A6F) for OX3E9D4A6F in OX7E2B5C3F]):
        return None
    OX3E9D4A6F = '/'.join(OX7E2B5C3F)
    return OX3E9D4A6F

class OX6F3C1D5B:
    def __init__(self, OX3E9D4A6F='x'):
        self.OX3E9D4A6F = OX3E9D4A6F

    def __call__(self, OX5E3B2D7A, OX7E2B5C3F, OX5D2E1F7C):
        OX7D5F1D2A = self.OX3A6F2D7B(OX5E3B2D7A, OX7E2B5C3F, OX5D2E1F7C)
        OX3C5B2D7F = OX5D2E1F7C.setdefault('_query', {})
        if isinstance(OX3C5B2D7F, dict):
            OX3C5B2D7F[self.OX3E9D4A6F] = OX7D5F1D2A
        else:
            OX5D2E1F7C['_query'] = tuple(OX3C5B2D7F) + ((self.OX3E9D4A6F, OX7D5F1D2A),)
        return OX7E2B5C3F, OX5D2E1F7C

class OX3F1B2D6E(OX6F3C1D5B):
    def __init__(self, OX3D2E7F6B, OX3E9D4A6F='x'):
        super().__init__(OX3E9D4A6F=OX3E9D4A6F)
        self._OX3D2E7F6B = OX3D2E7F6B

    def OX3A6F2D7B(self, OX5E3B2D7A, OX7E2B5C3F, OX5D2E1F7C):
        return self._OX3D2E7F6B

class OX6C7F2E3D:
    OX6D98D9C9 = staticmethod(OX6D98D9C9)
    OX2BC8A7E5 = staticmethod(OX2BC8A7E5)

    def __init__(self, OX5A2EB61D, OX7F6B4C1D=False):
        OX2F7C3B6A = OX49E6D5D3().__name__
        self.OX3E9D4A6F = OX5A2EB61D(OX5A2EB61D, OX2F7C3B6A)
        self.OX7F6B4C1D = OX7F6B4C1D
        self._OX2BC8A7E5 = None
        if not OX7F6B4C1D:
            self._OX3A6F2D7B = self.OX3C5B2D7F()

    def OX3C5B2D7F(self):
        with open(self.OX3E9D4A6F, 'rb') as OX7D5F1D2A:
            return self.OX2E6D7F3B(OX7D5F1D2A.read())

    def OX2E6D7F3B(self, OX5E3B2D7A):
        return OX7EC8BD5D.loads(OX5E3B2D7A.decode('utf-8'))

    @property
    def OX3A6F2D7B(self):
        if self.OX7F6B4C1D:
            if not self.OX6D98D9C9(self.OX3E9D4A6F):
                return {}
            OX2BC8A7E5 = self.OX2BC8A7E5(self.OX3E9D4A6F)
            if self._OX2BC8A7E5 is None or OX2BC8A7E5 > self._OX2BC8A7E5:
                self._OX3A6F2D7B = self.OX3C5B2D7F()
                self._OX2BC8A7E5 = OX2BC8A7E5
        return self._OX3A6F2D7B

    def __call__(self, OX5E3B2D7A, OX7E2B5C3F, OX5D2E1F7C):
        OX7E2B5C3F = self.OX3A6F2D7B.get(OX7E2B5C3F, OX7E2B5C3F)
        return (OX7E2B5C3F, OX5D2E1F7C)