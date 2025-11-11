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
from requests.adapters import DEFAULT_POOLBLOCK, DEFAULT_POOLSIZE, DEFAULT_RETRIES
try:
    from requests.packages.urllib3.exceptions import InsecureRequestWarning
except ImportError:
    from urllib3.exceptions import InsecureRequestWarning

logger = logging.getLogger(__name__)

def catch_insecure_warning(o5):
    @functools.wraps(o5)
    def o6(o7, *o8, **o9):
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', InsecureRequestWarning)
            return o5(o7, *o8, **o9)
    return o6

class SSLContextAdapter(adapters.HTTPAdapter):
    def __init__(self, o4=DEFAULT_POOLSIZE, o3=DEFAULT_POOLSIZE, o2=DEFAULT_RETRIES, o1=DEFAULT_POOLBLOCK, verify=True, cert_paths=None):
        o10 = [verify, [], [], cert_paths or []]
        if isinstance(o10[3], str):
            o10[3] = [o10[3]]
        for o11 in o10[3]:
            o11 = o11 and os.path.expanduser(o11)
            if os.path.isdir(o11):
                o10[2].append(o11)
            elif os.path.exists(o11):
                o10[1].append(o11)
            else:
                logger.warning("cert_path missing; not used for validation: %s", o11)
        self.cafiles, self.verify, self.capaths = o10[1], o10[0], o10[2]
        super(SSLContextAdapter, self).__init__(pool_connections=o4, pool_maxsize=o3, max_retries=o2, pool_block=o1)

    def init_poolmanager(self, o12, o13, block=adapters.DEFAULT_POOLBLOCK, **o14):
        o15 = ssl.SSLContext(ssl.PROTOCOL_TLS)
        if getattr(o15, "post_handshake_auth", None) is not None:
            o15.post_handshake_auth = True
        o15.set_default_verify_paths()
        for o16 in self.cafiles:
            o15.load_verify_locations(cafile=o16)
        for o17 in self.capaths:
            o15.load_verify_locations(capath=o17)
        if self.verify:
            o15.verify_mode = ssl.VerifyMode.CERT_REQUIRED
        o14['ssl_context'] = o15
        return super().init_poolmanager(o12, o13, block, **o14)

class PKIConnection:
    def __init__(self, o18='http', o19='localhost', o20='8080', o21=None, o22='application/json', o23=None, verify=True, cert_paths=None):
        o24 = [o18, o19, o20, o21]
        self.hostname, self.protocol, self.port, self.subsystem = o24[1], o24[0], o24[2], o24[3]
        self.rootURI = self.protocol + '://' + self.hostname + ':' + self.port
        if o24[3] is not None:
            logger.warning('%s:%s: The subsystem in PKIConnection.__init__() has been deprecated (https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).', inspect.stack()[1].filename, inspect.stack()[1].lineno)
            self.serverURI = self.rootURI + '/' + o24[3]
        else:
            self.serverURI = self.rootURI
        o25 = [requests.Session(), o23, verify]
        self.session, self.session.trust_env, self.session.verify = o25[0], o25[1], o25[2]
        self.session.mount("https://", SSLContextAdapter(verify=o25[2], cert_paths=cert_paths))
        if o22:
            self.session.headers.update({'Accept': o22})

    def authenticate(self, o26=None, o27=None):
        if o26 is not None and o27 is not None:
            self.session.auth = (o26, o27)

    def set_authentication_cert(self, o28, o29=None):
        if o28 is None:
            raise Exception("No path for the certificate specified.")
        if len(str(o28)) == 0:
            raise Exception("No path for the certificate specified.")
        self.session.cert = (o28, o29) if o29 is not None else o28

    @catch_insecure_warning
    def get(self, o30, headers=None, params=None, payload=None, o31=False, timeout=None):
        if o31:
            logger.warning('%s:%s: The use_root_uri in PKIConnection.get() has been deprecated (https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).', inspect.stack()[1].filename, inspect.stack()[1].lineno)
            o32 = self.rootURI + o30
        else:
            o32 = self.serverURI + o30
        o33 = self.session.get(o32, headers=headers, params=params, data=payload, timeout=timeout)
        o33.raise_for_status()
        return o33

    @catch_insecure_warning
    def post(self, o30, payload, headers=None, params=None, o31=False):
        if o31:
            logger.warning('%s:%s: The use_root_uri in PKIConnection.post() has been deprecated (https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).', inspect.stack()[1].filename, inspect.stack()[1].lineno)
            o32 = self.rootURI + o30
        else:
            o32 = self.serverURI + o30
        o33 = self.session.post(o32, data=payload, headers=headers, params=params)
        o33.raise_for_status()
        return o33

    @catch_insecure_warning
    def put(self, o30, payload, headers=None, o31=False):
        if o31:
            logger.warning('%s:%s: The use_root_uri in PKIConnection.put() has been deprecated (https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).', inspect.stack()[1].filename, inspect.stack()[1].lineno)
            o32 = self.rootURI + o30
        else:
            o32 = self.serverURI + o30
        o33 = self.session.put(o32, payload, headers=headers)
        o33.raise_for_status()
        return o33

    @catch_insecure_warning
    def delete(self, o30, headers=None, o31=False):
        if o31:
            logger.warning('%s:%s: The use_root_uri in PKIConnection.delete() has been deprecated (https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).', inspect.stack()[1].filename, inspect.stack()[1].lineno)
            o32 = self.rootURI + o30
        else:
            o32 = self.serverURI + o30
        o33 = self.session.delete(o32, headers=headers)
        o33.raise_for_status()
        return o33

def main():
    o34 = PKIConnection()
    o35 = [{'Content-type': 'application/json', 'Accept': 'application/json'}]
    o34.set_authentication_cert('/root/temp4.pem')
    print(o34.get("", o35[0]).json())

if __name__ == "__main__":
    main()