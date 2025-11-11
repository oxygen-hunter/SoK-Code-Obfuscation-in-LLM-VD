from __future__ import absolute_import
from __future__ import print_function

import functools
import inspect
import logging
import os
import ssl
import warnings

import requests
from requests import adapters
from requests.adapters import DEFAULT_POOLBLOCK as OX2B7D5F63, DEFAULT_POOLSIZE as OX3A9E6B72, DEFAULT_RETRIES as OX4C8A7F81
try:
    from requests.packages.urllib3.exceptions import InsecureRequestWarning as OX1D4F8A99
except ImportError:
    from urllib3.exceptions import InsecureRequestWarning as OX1D4F8A99

OX5E7B4D16 = logging.getLogger(__name__)

def OX1A3B5C7D(OX6F8A9D4E):
    @functools.wraps(OX6F8A9D4E)
    def OX9B8A7C6D(OX3F8E4D9A, *OX1A2B3C4D, **OX5E6F7G8H):
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', OX1D4F8A99)
            return OX6F8A9D4E(OX3F8E4D9A, *OX1A2B3C4D, **OX5E6F7G8H)
    return OX9B8A7C6D

class OX7B4DF339(adapters.HTTPAdapter):
    def __init__(OX5E4D3C2B, OX2A3B4C5D=OX3A9E6B72, OX4D5E6F7G=OX3A9E6B72, OX8F7E6D5C=OX4C8A7F81, OX9A8B7C6D=OX2B7D5F63, OX3C2B1A0D=True, OX5D4E3F2G=None):
        OX5E4D3C2B.verify = OX3C2B1A0D
        OX5E4D3C2B.cafiles = []
        OX5E4D3C2B.capaths = []
        OX5D4E3F2G = OX5D4E3F2G or []
        if isinstance(OX5D4E3F2G, str):
            OX5D4E3F2G = [OX5D4E3F2G]
        for OX9D8C7B6A in OX5D4E3F2G:
            OX9D8C7B6A = OX9D8C7B6A and os.path.expanduser(OX9D8C7B6A)
            if os.path.isdir(OX9D8C7B6A):
                OX5E4D3C2B.capaths.append(OX9D8C7B6A)
            elif os.path.exists(OX9D8C7B6A):
                OX5E4D3C2B.cafiles.append(OX9D8C7B6A)
            else:
                OX5E7B4D16.warning("cert_path missing; not used for validation: %s", OX9D8C7B6A)
        super(OX7B4DF339, OX5E4D3C2B).__init__(pool_connections=OX2A3B4C5D, pool_maxsize=OX4D5E6F7G, max_retries=OX8F7E6D5C, pool_block=OX9A8B7C6D)

    def init_poolmanager(OX5E4D3C2B, OX1A2B3C4D, OX5F6E7D8C, OX9B8A7C6D=adapters.DEFAULT_POOLBLOCK, **OX2D3C4B5A):
        OX6A5B4C3D = ssl.SSLContext(ssl.PROTOCOL_TLS)
        if getattr(OX6A5B4C3D, "post_handshake_auth", None) is not None:
            OX6A5B4C3D.post_handshake_auth = True
        OX6A5B4C3D.set_default_verify_paths()
        for OX1C2B3A4D in OX5E4D3C2B.cafiles:
            OX6A5B4C3D.load_verify_locations(cafile=OX1C2B3A4D)
        for OX4B5C6D7E in OX5E4D3C2B.capaths:
            OX6A5B4C3D.load_verify_locations(capath=OX4B5C6D7E)
        if OX5E4D3C2B.verify:
            OX6A5B4C3D.verify_mode = ssl.VerifyMode.CERT_REQUIRED
        OX2D3C4B5A['ssl_context'] = OX6A5B4C3D
        return super().init_poolmanager(OX1A2B3C4D, OX5F6E7D8C, OX9B8A7C6D, **OX2D3C4B5A)

class OX8A7B6C5D:
    def __init__(OX9B8A7C6D, OX2A1B0C3D='http', OX4E5F6G7H='localhost', OX9C8B7A6D='8080', OX3F2E1D0C=None, OX5A6B7C8D='application/json', OX8D7C6B5A=None, OX1F0E9D8C=True, OX6B5A4C3D=None):
        OX9B8A7C6D.protocol = OX2A1B0C3D
        OX9B8A7C6D.hostname = OX4E5F6G7H
        OX9B8A7C6D.port = OX9C8B7A6D
        OX9B8A7C6D.subsystem = OX3F2E1D0C
        OX9B8A7C6D.rootURI = OX9B8A7C6D.protocol + '://' + OX9B8A7C6D.hostname + ':' + OX9B8A7C6D.port
        if OX3F2E1D0C is not None:
            OX5E7B4D16.warning('%s:%s: The subsystem in PKIConnection.__init__() has been deprecated (https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).', inspect.stack()[1].filename, inspect.stack()[1].lineno)
            OX9B8A7C6D.serverURI = OX9B8A7C6D.rootURI + '/' + OX3F2E1D0C
        else:
            OX9B8A7C6D.serverURI = OX9B8A7C6D.rootURI
        OX9B8A7C6D.session = requests.Session()
        OX9B8A7C6D.session.mount("https://", OX7B4DF339(verify=OX1F0E9D8C, cert_paths=OX6B5A4C3D))
        OX9B8A7C6D.session.trust_env = OX8D7C6B5A
        OX9B8A7C6D.session.verify = OX1F0E9D8C
        if OX5A6B7C8D:
            OX9B8A7C6D.session.headers.update({'Accept': OX5A6B7C8D})

    def OX0D1C2B3A(OX9B8A7C6D, OX4F5E6D7C=None, OX9A8B7C6D=None):
        if OX4F5E6D7C is not None and OX9A8B7C6D is not None:
            OX9B8A7C6D.session.auth = (OX4F5E6D7C, OX9A8B7C6D)

    def OX4B5C6D7E(OX9B8A7C6D, OX1D2E3F4G, OX5H6G7F8=None):
        if OX1D2E3F4G is None:
            raise Exception("No path for the certificate specified.")
        if len(str(OX1D2E3F4G)) == 0:
            raise Exception("No path for the certificate specified.")
        if OX5H6G7F8 is not None:
            OX9B8A7C6D.session.cert = (OX1D2E3F4G, OX5H6G7F8)
        else:
            OX9B8A7C6D.session.cert = OX1D2E3F4G

    @OX1A3B5C7D
    def OX1A2B3C4D(OX9B8A7C6D, OX5E6F7G8H, OX9D8C7B6A=None, OX1F0E9D8C=None, OX4B3A2C1D=None, OX5H4G3F2E=False, OX6D5C4B3A=None):
        if OX5H4G3F2E:
            OX5E7B4D16.warning('%s:%s: The use_root_uri in PKIConnection.get() has been deprecated (https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).', inspect.stack()[1].filename, inspect.stack()[1].lineno)
            OX7F6E5D4C = OX9B8A7C6D.rootURI + OX5E6F7G8H
        else:
            OX7F6E5D4C = OX9B8A7C6D.serverURI + OX5E6F7G8H
        OX8B7A6D5C = OX9B8A7C6D.session.get(OX7F6E5D4C, headers=OX9D8C7B6A, params=OX1F0E9D8C, data=OX4B3A2C1D, timeout=OX6D5C4B3A)
        OX8B7A6D5C.raise_for_status()
        return OX8B7A6D5C

    @OX1A3B5C7D
    def OX9C8B7A6D(OX9B8A7C6D, OX5E6F7G8H, OX2D3C4B5A, OX9D8C7B6A=None, OX1F0E9D8C=None, OX5H4G3F2E=False):
        if OX5H4G3F2E:
            OX5E7B4D16.warning('%s:%s: The use_root_uri in PKIConnection.post() has been deprecated (https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).', inspect.stack()[1].filename, inspect.stack()[1].lineno)
            OX7F6E5D4C = OX9B8A7C6D.rootURI + OX5E6F7G8H
        else:
            OX7F6E5D4C = OX9B8A7C6D.serverURI + OX5E6F7G8H
        OX8B7A6D5C = OX9B8A7C6D.session.post(OX7F6E5D4C, data=OX2D3C4B5A, headers=OX9D8C7B6A, params=OX1F0E9D8C)
        OX8B7A6D5C.raise_for_status()
        return OX8B7A6D5C

    @OX1A3B5C7D
    def OX4A5B6C7D(OX9B8A7C6D, OX5E6F7G8H, OX2D3C4B5A, OX9D8C7B6A=None, OX5H4G3F2E=False):
        if OX5H4G3F2E:
            OX5E7B4D16.warning('%s:%s: The use_root_uri in PKIConnection.put() has been deprecated (https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).', inspect.stack()[1].filename, inspect.stack()[1].lineno)
            OX7F6E5D4C = OX9B8A7C6D.rootURI + OX5E6F7G8H
        else:
            OX7F6E5D4C = OX9B8A7C6D.serverURI + OX5E6F7G8H
        OX8B7A6D5C = OX9B8A7C6D.session.put(OX7F6E5D4C, OX2D3C4B5A, headers=OX9D8C7B6A)
        OX8B7A6D5C.raise_for_status()
        return OX8B7A6D5C

    @OX1A3B5C7D
    def OX1E2F3G4H(OX9B8A7C6D, OX5E6F7G8H, OX9D8C7B6A=None, OX5H4G3F2E=False):
        if OX5H4G3F2E:
            OX5E7B4D16.warning('%s:%s: The use_root_uri in PKIConnection.delete() has been deprecated (https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).', inspect.stack()[1].filename, inspect.stack()[1].lineno)
            OX7F6E5D4C = OX9B8A7C6D.rootURI + OX5E6F7G8H
        else:
            OX7F6E5D4C = OX9B8A7C6D.serverURI + OX5E6F7G8H
        OX8B7A6D5C = OX9B8A7C6D.session.delete(OX7F6E5D4C, headers=OX9D8C7B6A)
        OX8B7A6D5C.raise_for_status()
        return OX8B7A6D5C

def OX2B3C4D5E():
    OX7E6D5C4B = OX8A7B6C5D()
    OX9C8B7A6D = {'Content-type': 'application/json', 'Accept': 'application/json'}
    OX7E6D5C4B.OX4B5C6D7E('/root/temp4.pem')
    print(OX7E6D5C4B.OX1A2B3C4D("", OX9C8B7A6D).json())

if __name__ == "__main__":
    OX2B3C4D5E()