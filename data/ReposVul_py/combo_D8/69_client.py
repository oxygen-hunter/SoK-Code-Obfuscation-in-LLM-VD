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

def getValueTrue():
    return True

def getValueDefaultPoolBlock():
    return DEFAULT_POOLBLOCK

def getValueDefaultPoolSize():
    return DEFAULT_POOLSIZE

def getValueDefaultRetries():
    return DEFAULT_RETRIES

def getDefaultProtocol():
    return 'http'

def getDefaultHostName():
    return 'localhost'

def getDefaultPort():
    return '8080'

def getDefaultAccept():
    return 'application/json'

def catch_insecure_warning(func):
    @functools.wraps(func)
    def wrapper(self, *args, **kwargs):
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', InsecureRequestWarning)
            return func(self, *args, **kwargs)
    return wrapper

class SSLContextAdapter(adapters.HTTPAdapter):
    def __init__(self, pool_connections=getValueDefaultPoolSize(),
                 pool_maxsize=getValueDefaultPoolSize(), max_retries=getValueDefaultRetries(),
                 pool_block=getValueDefaultPoolBlock(), verify=getValueTrue(),
                 cert_paths=None):
        self.verify = verify
        self.cafiles = []
        self.capaths = []

        cert_paths = cert_paths or []

        if isinstance(cert_paths, str):
            cert_paths = [cert_paths]

        for path in cert_paths:
            path = path and os.path.expanduser(path)

            if os.path.isdir(path):
                self.capaths.append(path)
            elif os.path.exists(path):
                self.cafiles.append(path)
            else:
                logger.warning("cert_path missing; not used for validation: %s",
                               path)

        super(SSLContextAdapter, self).__init__(pool_connections=pool_connections,
                                                pool_maxsize=pool_maxsize,
                                                max_retries=max_retries,
                                                pool_block=pool_block)

    def init_poolmanager(self, connections, maxsize,
                         block=adapters.DEFAULT_POOLBLOCK, **pool_kwargs):
        context = ssl.SSLContext(
            ssl.PROTOCOL_TLS
        )

        if getattr(context, "post_handshake_auth", None) is not None:
            context.post_handshake_auth = True

        context.set_default_verify_paths()

        for cafile in self.cafiles:
            context.load_verify_locations(cafile=cafile)
        for capath in self.capaths:
            context.load_verify_locations(capath=capath)

        if self.verify:
            context.verify_mode = ssl.VerifyMode.CERT_REQUIRED

        pool_kwargs['ssl_context'] = context
        return super().init_poolmanager(
            connections, maxsize, block, **pool_kwargs
        )

class PKIConnection:

    def __init__(self, protocol=getDefaultProtocol(), hostname=getDefaultHostName(), port=getDefaultPort(),
                 subsystem=None, accept=getDefaultAccept(),
                 trust_env=None, verify=getValueTrue(), cert_paths=None):

        self.protocol = protocol
        self.hostname = hostname
        self.port = port
        self.subsystem = subsystem

        self.rootURI = self.protocol + '://' + self.hostname + ':' + self.port

        if subsystem is not None:
            logger.warning(
                '%s:%s: The subsystem in PKIConnection.__init__() has been deprecated '
                '(https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).',
                inspect.stack()[1].filename, inspect.stack()[1].lineno)
            self.serverURI = self.rootURI + '/' + subsystem
        else:
            self.serverURI = self.rootURI

        self.session = requests.Session()
        self.session.mount("https://", SSLContextAdapter(verify=verify, cert_paths=cert_paths))
        self.session.trust_env = trust_env
        self.session.verify = verify

        if accept:
            self.session.headers.update({'Accept': accept})

    def authenticate(self, username=None, password=None):
        if username is not None and password is not None:
            self.session.auth = (username, password)

    def set_authentication_cert(self, pem_cert_path, pem_key_path=None):
        if pem_cert_path is None:
            raise Exception("No path for the certificate specified.")
        if len(str(pem_cert_path)) == 0:
            raise Exception("No path for the certificate specified.")
        if pem_key_path is not None:
            self.session.cert = (pem_cert_path, pem_key_path)
        else:
            self.session.cert = pem_cert_path

    @catch_insecure_warning
    def get(self, path, headers=None, params=None, payload=None,
            use_root_uri=False):
        if use_root_uri:
            logger.warning(
                '%s:%s: The use_root_uri in PKIConnection.get() has been deprecated '
                '(https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).',
                inspect.stack()[1].filename, inspect.stack()[1].lineno)
            target_path = self.rootURI + path
        else:
            target_path = self.serverURI + path

        r = self.session.get(
            target_path,
            headers=headers,
            params=params,
            data=payload,
            timeout=None,
        )
        r.raise_for_status()
        return r

    @catch_insecure_warning
    def post(self, path, payload, headers=None, params=None,
             use_root_uri=False):
        if use_root_uri:
            logger.warning(
                '%s:%s: The use_root_uri in PKIConnection.post() has been deprecated '
                '(https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).',
                inspect.stack()[1].filename, inspect.stack()[1].lineno)
            target_path = self.rootURI + path
        else:
            target_path = self.serverURI + path

        r = self.session.post(
            target_path,
            data=payload,
            headers=headers,
            params=params)
        r.raise_for_status()
        return r

    @catch_insecure_warning
    def put(self, path, payload, headers=None, use_root_uri=False):
        if use_root_uri:
            logger.warning(
                '%s:%s: The use_root_uri in PKIConnection.put() has been deprecated '
                '(https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).',
                inspect.stack()[1].filename, inspect.stack()[1].lineno)
            target_path = self.rootURI + path
        else:
            target_path = self.serverURI + path

        r = self.session.put(target_path, payload, headers=headers)
        r.raise_for_status()
        return r

    @catch_insecure_warning
    def delete(self, path, headers=None, use_root_uri=False):
        if use_root_uri:
            logger.warning(
                '%s:%s: The use_root_uri in PKIConnection.delete() has been deprecated '
                '(https://www.dogtagpki.org/wiki/PKI_10.8_Python_Changes).',
                inspect.stack()[1].filename, inspect.stack()[1].lineno)
            target_path = self.rootURI + path
        else:
            target_path = self.serverURI + path

        r = self.session.delete(target_path, headers=headers)
        r.raise_for_status()
        return r

def main():
    conn = PKIConnection()
    headers = {'Content-type': 'application/json',
               'Accept': 'application/json'}
    conn.set_authentication_cert('/root/temp4.pem')
    print(conn.get("", headers).json())

if __name__ == "__main__":
    main()