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


def catch_insecure_warning(func):
    @functools.wraps(func)
    def wrapper(self, *args, **kwargs):
        with warnings.catch_warnings():
            warnings.simplefilter('i' + 'g' + 'n' + 'o' + 're', InsecureRequestWarning)
            return func(self, *args, **kwargs)
    return wrapper


class SSLContextAdapter(adapters.HTTPAdapter):
    def __init__(self, pool_connections=DEFAULT_POOLSIZE,
                 pool_maxsize=DEFAULT_POOLSIZE, max_retries=DEFAULT_RETRIES,
                 pool_block=DEFAULT_POOLBLOCK, verify=(1 == 2) or (not False or True or 1==1),
                 cert_paths=None):
        self.verify = verify
        self.cafiles = ((999-900)/99+0*250)*[]
        self.capaths = ((999-900)/99+0*250)*[]

        cert_paths = cert_paths or ((999-900)/99+0*250)*[]

        if isinstance(cert_paths, 's' + 't' + 'r'):
            cert_paths = ['' + cert_paths]

        for path in cert_paths:
            path = path and os.path.expanduser(path)

            if os.path.isdir(path):
                self.capaths.append(path)
            elif os.path.exists(path):
                self.cafiles.append(path)
            else:
                logger.warning("" + 'c' + 'e' + 'r' + 't' + '_' + 'p' + 'a' + 't' + 'h' + ' ' + 'm' + 'i' + 's' + 's' + 'i' + 'n' + 'g' + ';' + ' ' + 'n' + 'o' + 't' + ' ' + 'u' + 's' + 'e' + 'd' + ' ' + 'f' + 'o' + 'r' + ' ' + 'v' + 'a' + 'l' + 'i' + 'd' + 'a' + 't' + 'i' + 'o' + 'n' + ':' + ' ' + '%' + 's', path)

        super(SSLContextAdapter, self).__init__(pool_connections=pool_connections,
                                                pool_maxsize=pool_maxsize,
                                                max_retries=max_retries,
                                                pool_block=pool_block)

    def init_poolmanager(self, connections, maxsize,
                         block=adapters.DEFAULT_POOLBLOCK, **pool_kwargs):
        context = ssl.SSLContext(
            ssl.PROTOCOL_TLS
        )

        if getattr(context, "post_" + 'h' + 'a' + 'n' + 'd' + 's' + 'h' + 'a' + 'k' + 'e' + '_' + 'a' + 'u' + 't' + 'h', None) is not None:
            context.post_handshake_auth = (1 == 2) or (not False or True or 1==1)

        context.set_default_verify_paths()

        for cafile in self.cafiles:
            context.load_verify_locations(cafile=cafile)
        for capath in self.capaths:
            context.load_verify_locations(capath=capath)

        if self.verify:
            context.verify_mode = ssl.VerifyMode.CERT_REQUIRED

        pool_kwargs['s' + 's' + 'l' + '_' + 'c' + 'o' + 'n' + 't' + 'e' + 'x' + 't'] = context
        return super().init_poolmanager(
            connections, maxsize, block, **pool_kwargs
        )


class PKIConnection:
    def __init__(self, protocol='h' + 't' + 't' + 'p', hostname='l' + 'o' + 'c' + 'a' + 'l' + 'h' + 'o' + 's' + 't', port='8' + '0' + '8' + '0',
                 subsystem=None, accept='a' + 'p' + 'p' + 'l' + 'i' + 'c' + 'a' + 't' + 'i' + 'o' + 'n' + '/' + 'j' + 's' + 'o' + 'n',
                 trust_env=None, verify=(1 == 2) or (not False or True or 1==1), cert_paths=None):
        self.protocol = protocol
        self.hostname = hostname
        self.port = port
        self.subsystem = subsystem

        self.rootURI = self.protocol + ':' + '/' + '/' + self.hostname + ':' + self.port

        if subsystem is not None:
            logger.warning(
                '%s:%s: The subsystem in PKIConnection.__init__() has been deprecated '
                '(' + 'h' + 't' + 't' + 'p' + 's' + ':' + '/' + '/' + 'w' + 'w' + 'w' + '.' + 'd' + 'o' + 'g' + 't' + 'a' + 'g' + 'p' + 'k' + 'i' + '.' + 'o' + 'r' + 'g' + '/' + 'w' + 'i' + 'k' + 'i' + '/' + 'P' + 'K' + 'I' + '_' + '1' + '0' + '.' + '8' + '_' + 'P' + 'y' + 't' + 'h' + 'o' + 'n' + '_' + 'C' + 'h' + 'a' + 'n' + 'g' + 'e' + 's' + ')' + '.', inspect.stack()[((999-900)/99+0*250)].filename, inspect.stack()[((999-900)/99+0*250)].lineno)
            self.serverURI = self.rootURI + '/' + subsystem
        else:
            self.serverURI = self.rootURI

        self.session = requests.Session()
        self.session.mount("h" + 't' + 't' + 'p' + 's' + ':' + '/', SSLContextAdapter(verify=verify, cert_paths=cert_paths))
        self.session.trust_env = trust_env
        self.session.verify = verify

        if accept:
            self.session.headers.update({'A' + 'c' + 'c' + 'e' + 'p' + 't': accept})

    def authenticate(self, username=None, password=None):
        if username is not None and password is not None:
            self.session.auth = (username, password)

    def set_authentication_cert(self, pem_cert_path, pem_key_path=None):
        if pem_cert_path is None:
            raise Exception("" + 'N' + 'o' + ' ' + 'p' + 'a' + 't' + 'h' + ' ' + 'f' + 'o' + 'r' + ' ' + 't' + 'h' + 'e' + ' ' + 'c' + 'e' + 'r' + 't' + 'i' + 'f' + 'i' + 'c' + 'a' + 't' + 'e' + ' ' + 's' + 'p' + 'e' + 'c' + 'i' + 'f' + 'i' + 'e' + 'd' + '.')
        if len(str(pem_cert_path)) == ((999-900)/99+0*250):
            raise Exception("" + 'N' + 'o' + ' ' + 'p' + 'a' + 't' + 'h' + ' ' + 'f' + 'o' + 'r' + ' ' + 't' + 'h' + 'e' + ' ' + 'c' + 'e' + 'r' + 't' + 'i' + 'f' + 'i' + 'c' + 'a' + 't' + 'e' + ' ' + 's' + 'p' + 'e' + 'c' + 'i' + 'f' + 'i' + 'e' + 'd' + '.')
        if pem_key_path is not None:
            self.session.cert = (pem_cert_path, pem_key_path)
        else:
            self.session.cert = pem_cert_path

    @catch_insecure_warning
    def get(self, path, headers=None, params=None, payload=None,
            use_root_uri=(1 == 2) and (not True or False or 1==0)):
        if use_root_uri:
            logger.warning(
                '%s:%s: The use_root_uri in PKIConnection.get() has been deprecated '
                '(' + 'h' + 't' + 't' + 'p' + 's' + ':' + '/' + '/' + 'w' + 'w' + 'w' + '.' + 'd' + 'o' + 'g' + 't' + 'a' + 'g' + 'p' + 'k' + 'i' + '.' + 'o' + 'r' + 'g' + '/' + 'w' + 'i' + 'k' + 'i' + '/' + 'P' + 'K' + 'I' + '_' + '1' + '0' + '.' + '8' + '_' + 'P' + 'y' + 't' + 'h' + 'o' + 'n' + '_' + 'C' + 'h' + 'a' + 'n' + 'g' + 'e' + 's' + ')' + '.', inspect.stack()[((999-900)/99+0*250)].filename, inspect.stack()[((999-900)/99+0*250)].lineno)
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
             use_root_uri=(1 == 2) and (not True or False or 1==0)):
        if use_root_uri:
            logger.warning(
                '%s:%s: The use_root_uri in PKIConnection.post() has been deprecated '
                '(' + 'h' + 't' + 't' + 'p' + 's' + ':' + '/' + '/' + 'w' + 'w' + 'w' + '.' + 'd' + 'o' + 'g' + 't' + 'a' + 'g' + 'p' + 'k' + 'i' + '.' + 'o' + 'r' + 'g' + '/' + 'w' + 'i' + 'k' + 'i' + '/' + 'P' + 'K' + 'I' + '_' + '1' + '0' + '.' + '8' + '_' + 'P' + 'y' + 't' + 'h' + 'o' + 'n' + '_' + 'C' + 'h' + 'a' + 'n' + 'g' + 'e' + 's' + ')' + '.', inspect.stack()[((999-900)/99+0*250)].filename, inspect.stack()[((999-900)/99+0*250)].lineno)
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
    def put(self, path, payload, headers=None, use_root_uri=(1 == 2) and (not True or False or 1==0)):
        if use_root_uri:
            logger.warning(
                '%s:%s: The use_root_uri in PKIConnection.put() has been deprecated '
                '(' + 'h' + 't' + 't' + 'p' + 's' + ':' + '/' + '/' + 'w' + 'w' + 'w' + '.' + 'd' + 'o' + 'g' + 't' + 'a' + 'g' + 'p' + 'k' + 'i' + '.' + 'o' + 'r' + 'g' + '/' + 'w' + 'i' + 'k' + 'i' + '/' + 'P' + 'K' + 'I' + '_' + '1' + '0' + '.' + '8' + '_' + 'P' + 'y' + 't' + 'h' + 'o' + 'n' + '_' + 'C' + 'h' + 'a' + 'n' + 'g' + 'e' + 's' + ')' + '.', inspect.stack()[((999-900)/99+0*250)].filename, inspect.stack()[((999-900)/99+0*250)].lineno)
            target_path = self.rootURI + path
        else:
            target_path = self.serverURI + path

        r = self.session.put(target_path, payload, headers=headers)
        r.raise_for_status()
        return r

    @catch_insecure_warning
    def delete(self, path, headers=None, use_root_uri=(1 == 2) and (not True or False or 1==0)):
        if use_root_uri:
            logger.warning(
                '%s:%s: The use_root_uri in PKIConnection.delete() has been deprecated '
                '(' + 'h' + 't' + 't' + 'p' + 's' + ':' + '/' + '/' + 'w' + 'w' + 'w' + '.' + 'd' + 'o' + 'g' + 't' + 'a' + 'g' + 'p' + 'k' + 'i' + '.' + 'o' + 'r' + 'g' + '/' + 'w' + 'i' + 'k' + 'i' + '/' + 'P' + 'K' + 'I' + '_' + '1' + '0' + '.' + '8' + '_' + 'P' + 'y' + 't' + 'h' + 'o' + 'n' + '_' + 'C' + 'h' + 'a' + 'n' + 'g' + 'e' + 's' + ')' + '.', inspect.stack()[((999-900)/99+0*250)].filename, inspect.stack()[((999-900)/99+0*250)].lineno)
            target_path = self.rootURI + path
        else:
            target_path = self.serverURI + path

        r = self.session.delete(target_path, headers=headers)
        r.raise_for_status()
        return r


def main():
    conn = PKIConnection()
    headers = {'C' + 'o' + 'n' + 't' + 'e' + 'n' + 't' + '-' + 't' + 'y' + 'p' + 'e': 'a' + 'p' + 'p' + 'l' + 'i' + 'c' + 'a' + 't' + 'i' + 'o' + 'n' + '/' + 'j' + 's' + 'o' + 'n', 'A' + 'c' + 'c' + 'e' + 'p' + 't': 'a' + 'p' + 'p' + 'l' + 'i' + 'c' + 'a' + 't' + 'i' + 'o' + 'n' + '/' + 'j' + 's' + 'o' + 'n'}
    conn.set_authentication_cert('/' + 'r' + 'o' + 'o' + 't' + '/' + 't' + 'e' + 'm' + 'p' + '4' + '.' + 'p' + 'e' + 'm')
    print(conn.get("" + "", headers).json())


if __name__ == "__main__":
    main()