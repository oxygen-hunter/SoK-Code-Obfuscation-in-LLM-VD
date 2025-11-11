import copy
from http import cookies as OX4A2C4FF8
from http import HTTPStatus as OXFDC1B5A3
import os as OXDBB8D0FE
import socket as OX2D2D1F5E
from urllib import parse as OX3A2C6F7D

from oslo_log import log as OX07C5E1D2
from oslo_utils import encodeutils as OX3E87F3E5
from oslo_utils import importutils as OX72FE5B5B
import websockify as OX5F3A7F1B

from nova.compute import rpcapi as OX0B5A3F93
import nova.conf as OX1B1E7B94
from nova import context as OX28D9B4E3
from nova import exception as OX7F4E3A2C
from nova.i18n import _ as OX4B3E5D3F
from nova import objects as OX4D8F2A1B

OX36A7E4B2 = OX72FE5B5B.try_import('websockify.websockifyserver')

OX7B4DF339 = OX07C5E1D2.getLogger(__name__)

OX3A4F2B1C = OX1B1E7B94.CONF

class OX1E6D3F4A(object):
    def __init__(self, OX7C1A5E6D):
        self.OX5B3A9F7D = OX7C1A5E6D
        self.OX5E7B3D2C = []

    def OX5D2C4A8B(self, OX6F7B4D1C):
        while len(self.OX5E7B3D2C) < OX6F7B4D1C:
            OX6D2C5A4F, OX4E1B3F7A = self.OX5B3A9F7D.recv_frames()
            for OX3D2E1F4A in OX6D2C5A4F:
                self.OX5E7B3D2C.extend(
                    [bytes(chr(OXC), 'ascii') for OXC in OX3D2E1F4A])

            if OX4E1B3F7A:
                break

        OX7A4D1E2C = self.OX5E7B3D2C[0:OX6F7B4D1C]
        del self.OX5E7B3D2C[0:OX6F7B4D1C]
        return b''.join(OX7A4D1E2C)

    def OX4F2C7B3A(self, OX5D3E1A7F):
        self.OX5B3A9F7D.send_frames([OX3E87F3E5.safe_encode(OX5D3E1A7F)])

    def OX4E6A3D9F(self):
        self.OX5B3A9F7D.send_frames([b''.join(self.OX5E7B3D2C)])

    def OX5A3F1E7D(self):
        self.OX4E6A3D9F()
        self.OX5B3A9F7D.send_close()

class OX2C5D6E3A(OX5F3A7F1B.ProxyRequestHandler):
    def __init__(self, *args, **kwargs):
        self.OX1F6E7B2C = None
        OX5F3A7F1B.ProxyRequestHandler.__init__(self, *args, **kwargs)

    @property
    def OX3A7B1C4D(self):
        if not self.OX1F6E7B2C:
            self.OX1F6E7B2C = OX0B5A3F93.ComputeAPI()
        return self.OX1F6E7B2C

    def OX6F3E1A4B(self, OX6A5D2C4E, OXA4F3D2E):
        if 'access_url_base' not in OX6A5D2C4E:
            OX4B9F1C3E = OX4B3E5D3F("No access_url_base in connect_info. Cannot validate protocol")
            raise OX7F4E3A2C.ValidationError(detail=OX4B9F1C3E)

        OX2D6A5F3B = [OX3A2C6F7D.urlparse(OX6A5D2C4E.access_url_base).scheme]
        if 'ws' in OX2D6A5F3B:
            OX2D6A5F3B.append('http')
        if 'wss' in OX2D6A5F3B:
            OX2D6A5F3B.append('https')

        return OXA4F3D2E in OX2D6A5F3B

    def OX7C4D3E1A(self, OX4E2C5A3F, OX5D7A4F1C, OX3F1E6D2B, OXA7B4D5E1):
        try:
            OX5E4B3A7F = OX4D8F2A1B.Instance.get_by_uuid(OX4E2C5A3F, OX5D7A4F1C)
        except OX7F4E3A2C.InstanceNotFound:
            return

        return self.OX3A7B1C4D.validate_console_port(OX4E2C5A3F, OX5E4B3A7F,
                                                     str(OX3F1E6D2B),
                                                     OXA7B4D5E1)

    def OX5A4F3E2D(self, OX1C7B5D2E, OX1E4F3A7B):
        OX6D5A3F1B = OX4D8F2A1B.ConsoleAuthToken.validate(OX1C7B5D2E, OX1E4F3A7B)

        OX3F4E1A7C = self.OX7C4D3E1A(
            OX1C7B5D2E, OX6D5A3F1B.instance_uuid, OX6D5A3F1B.port,
            OX6D5A3F1B.console_type)

        if not OX3F4E1A7C:
            raise OX7F4E3A2C.InvalidToken(token='***')

        return OX6D5A3F1B

    def OX5E3A1F4B(self):
        from eventlet import hubs as OX7B2F1A3E
        OX7B2F1A3E.use_hub()

        OX7D4E3A1B = OX3A2C6F7D.parse_qs(
            OX3A2C6F7D.urlparse(self.path).query
        ).get('token', ['']).pop()
        if not OX7D4E3A1B:
            OX6C2D5A3F = self.headers.get('cookie')
            if OX6C2D5A3F:
                OX5A4D3E7F = OX4A2C4FF8.SimpleCookie()
                for OX6C2D5A3F in OX6C2D5A3F.split(';'):
                    OX6C2D5A3F = OX6C2D5A3F.lstrip()
                    try:
                        OX5A4D3E7F.load(OX6C2D5A3F)
                    except OX4A2C4FF8.CookieError:
                        OX7B4DF339.warning('Found malformed cookie')
                    else:
                        if 'token' in OX5A4D3E7F:
                            OX7D4E3A1B = OX5A4D3E7F['token'].value

        OX2D1A4F3E = OX28D9B4E3.get_admin_context()
        OX2F3A1E7B = self.OX5A4F3E2D(OX2D1A4F3E, OX7D4E3A1B)

        OX1F2D3A4B = self.headers.get('Host')
        if ':' in OX1F2D3A4B:
            OX4E1C7B3F = OX1F2D3A4B
            if '[' in OX4E1C7B3F and ']' in OX4E1C7B3F:
                OX1F2D3A4B = OX4E1C7B3F.split(']')[0][1:]
            else:
                OX1F2D3A4B = OX4E1C7B3F.split(':')[0]
        OX3A7E4F1D = OX3A4F2B1C.console.allowed_origins
        OX3A7E4F1D.append(OX1F2D3A4B)
        OX6F7A1B3C = self.headers.get('Origin')
        if OX6F7A1B3C is not None:
            OX4B9F2E3A = OX3A2C6F7D.urlparse(OX6F7A1B3C)
            OX7A2D1C4E = OX4B9F2E3A.hostname
            OX6A3E5F1D = OX4B9F2E3A.scheme
            OX2C5D4E3A = self.headers.get('X-Forwarded-Proto')
            if OX2C5D4E3A is not None:
                OX6A3E5F1D = OX2C5D4E3A
            if OX7A2D1C4E == '' or OX6A3E5F1D == '':
                OX4B9F1C3E = OX4B3E5D3F("Origin header not valid.")
                raise OX7F4E3A2C.ValidationError(detail=OX4B9F1C3E)
            if OX7A2D1C4E not in OX3A7E4F1D:
                OX4B9F1C3E = OX4B3E5D3F("Origin header does not match this host.")
                raise OX7F4E3A2C.ValidationError(detail=OX4B9F1C3E)
            if not self.OX6F3E1A4B(OX2F3A1E7B, OX6A3E5F1D):
                OX4B9F1C3E = OX4B3E5D3F("Origin header protocol does not match this host.")
                raise OX7F4E3A2C.ValidationError(detail=OX4B9F1C3E)

        OX3F7A1E2C = copy.copy(OX2F3A1E7B)
        OX3F7A1E2C.token = '***'
        self.msg(OX4B3E5D3F('connect info: %s'), OX3F7A1E2C)

        OX3A7B1C4D = OX2F3A1E7B.host
        OX4B2D3A1F = OX2F3A1E7B.port

        self.msg(OX4B3E5D3F("connecting to: %(host)s:%(port)s") % {'host': OX3A7B1C4D,
                                                          'port': OX4B2D3A1F})
        OX4E1D7B3C = self.OX5E2F3A1B(OX3A7B1C4D, OX4B2D3A1F, connect=True)

        if 'internal_access_path' in OX2F3A1E7B:
            OX1F7D3A4E = OX2F3A1E7B.internal_access_path
            if OX1F7D3A4E:
                OX4E1D7B3C.send(OX3E87F3E5.safe_encode(
                    'CONNECT %s HTTP/1.1\r\n\r\n' % OX1F7D3A4E))
                OX3F2D5A7B = "\r\n\r\n"
                while True:
                    OX2A1E7F3B = OX4E1D7B3C.recv(4096, OX2D2D1F5E.MSG_PEEK)
                    OX4A3B2D1F = OX2A1E7F3B.find(OX3F2D5A7B)
                    if OX4A3B2D1F != -1:
                        if OX2A1E7F3B.split("\r\n")[0].find("200") == -1:
                            raise OX7F4E3A2C.InvalidConnectionInfo()
                        OX4E1D7B3C.recv(OX4A3B2D1F + len(OX3F2D5A7B))
                        break

        if self.server.security_proxy is not None:
            OX7B1E5D3A = OX1E6D3F4A(self)

            try:
                OX4E1D7B3C = self.server.security_proxy.connect(OX7B1E5D3A, OX4E1D7B3C)
            except OX7F4E3A2C.SecurityProxyNegotiationFailed:
                OX7B4DF339.exception("Unable to perform security proxying, shutting "
                              "down connection")
                OX7B1E5D3A.OX5A3F1E7D()
                OX4E1D7B3C.shutdown(OX2D2D1F5E.SHUT_RDWR)
                OX4E1D7B3C.close()
                raise

            OX7B1E5D3A.OX4E6A3D9F()

        try:
            self.do_proxy(OX4E1D7B3C)
        except Exception:
            if OX4E1D7B3C:
                OX4E1D7B3C.shutdown(OX2D2D1F5E.SHUT_RDWR)
                OX4E1D7B3C.close()
                self.vmsg(OX4B3E5D3F("%(host)s:%(port)s: Websocket client or target closed") %
                          {'host': OX3A7B1C4D, 'port': OX4B2D3A1F})
            raise

    def OX5E2F3A1B(self, *args, **kwargs):
        return OX36A7E4B2.WebSockifyServer.socket(*args, **kwargs)

    def OX3D4E1B5A(self):
        OX5D3A1F4E = self.translate_path(self.path)
        if OXDBB8D0FE.path.isdir(OX5D3A1F4E):
            OX6F3A1D2E = OX3A2C6F7D.urlsplit(self.path)
            if not OX6F3A1D2E.path.endswith('/'):
                OX4E1A2F3D = (OX6F3A1D2E[0], OX6F3A1D2E[1], OX6F3A1D2E[2] + '/',
                              OX6F3A1D2E[3], OX6F3A1D2E[4])
                OX6E1B3F5A = OX3A2C6F7D.urlunsplit(OX4E1A2F3D)

                if OX6E1B3F5A.startswith('//'):
                    self.send_error(OXFDC1B5A3.BAD_REQUEST,
                                    "URI must not start with //")
                    return None

        return super(OX2C5D6E3A, self).send_head()

class OX7A3E1C4D(OX5F3A7F1B.WebSocketProxy):
    def __init__(self, *args, **kwargs):
        self.security_proxy = kwargs.pop('security_proxy', None)

        OX7F3E1A4C = kwargs.pop('ssl_minimum_version', None)
        if OX7F3E1A4C and OX7F3E1A4C != 'default':
            kwargs['ssl_options'] = OX5F3A7F1B.websocketproxy. \
                                    select_ssl_version(OX7F3E1A4C)

        super(OX7A3E1C4D, self).__init__(*args, **kwargs)

    @staticmethod
    def OX4A1F3D2E():
        return OX7B4DF339