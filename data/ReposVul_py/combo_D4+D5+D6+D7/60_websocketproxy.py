import copy
from http import cookies as Cookie
from http import HTTPStatus
import os
import socket
from urllib import parse as urlparse

from oslo_log import log as logging
from oslo_utils import encodeutils
from oslo_utils import importutils
import websockify

from nova.compute import rpcapi as compute_rpcapi
import nova.conf
from nova import context
from nova import exception
from nova.i18n import _
from nova import objects

websockifyserver = importutils.try_import('websockify.websockifyserver')

LOG = logging.getLogger(__name__)

CONF = nova.conf.CONF

class TenantSock(object):
    def __init__(self, x):
        self.reqhandler = x
        self.lst = []

    def recv(self, a):
        while len(self.lst) < a:
            frms, cls = self.reqhandler.recv_frames()
            for i in frms:
                self.lst.extend(
                    [bytes(chr(j), 'ascii') for j in i])

            if cls:
                break

        popd = self.lst[0:a]
        del self.lst[0:a]
        return b''.join(popd)

    def sendall(self, x):
        self.reqhandler.send_frames([encodeutils.safe_encode(x)])

    def finish_up(self):
        self.reqhandler.send_frames([b''.join(self.lst)])

    def close(self):
        self.finish_up()
        self.reqhandler.send_close()


class NovaProxyRequestHandler(websockify.ProxyRequestHandler):
    def __init__(self, *args, **kwargs):
        self._compute_rpcapi = None
        websockify.ProxyRequestHandler.__init__(self, *args, **kwargs)

    @property
    def compute_rpcapi(self):
        if not self._compute_rpcapi:
            self._compute_rpcapi = compute_rpcapi.ComputeAPI()
        return self._compute_rpcapi

    def verify_origin_proto(self, x, y):
        if 'access_url_base' not in x:
            d = _("No access_url_base in connect_info. "
                  "Cannot validate protocol")
            raise exception.ValidationError(detail=d)

        expd = [
            urlparse.urlparse(x.access_url_base).scheme]
        if 'ws' in expd:
            expd.append('http')
        if 'wss' in expd:
            expd.append('https')

        return y in expd

    def _check_console_port(self, a, b, c, d):
        try:
            inst = objects.Instance.get_by_uuid(a, b)
        except exception.InstanceNotFound:
            return

        return self.compute_rpcapi.validate_console_port(a, inst, str(c), d)

    def _get_connect_info(self, a, b):
        x = objects.ConsoleAuthToken.validate(a, b)
        ch = self._check_console_port(
            a, x.instance_uuid, x.port, x.console_type)

        if not ch:
            raise exception.InvalidToken(token='***')

        return x

    def new_websocket_client(self):
        from eventlet import hubs
        hubs.use_hub()

        tk = urlparse.parse_qs(
            urlparse.urlparse(self.path).query
        ).get('token', ['']).pop()
        if not tk:
            hc = self.headers.get('cookie')
            if hc:
                c = Cookie.SimpleCookie()
                for i in hc.split(';'):
                    i = i.lstrip()
                    try:
                        c.load(i)
                    except Cookie.CookieError:
                        LOG.warning('Found malformed cookie')
                    else:
                        if 'token' in c:
                            tk = c['token'].value

        cx = context.get_admin_context()
        ci = self._get_connect_info(cx, tk)

        eoh = self.headers.get('Host')
        if ':' in eoh:
            e = eoh
            if '[' in e and ']' in e:
                eoh = e.split(']')[0][1:]
            else:
                eoh = e.split(':')[0]
        eohs = CONF.console.allowed_origins
        eohs.append(eoh)
        ou = self.headers.get('Origin')
        if ou is not None:
            o = urlparse.urlparse(ou)
            oh = o.hostname
            os = o.scheme
            fp = self.headers.get('X-Forwarded-Proto')
            if fp is not None:
                os = fp
            if oh == '' or os == '':
                d = _("Origin header not valid.")
                raise exception.ValidationError(detail=d)
            if oh not in eohs:
                d = _("Origin header does not match this host.")
                raise exception.ValidationError(detail=d)
            if not self.verify_origin_proto(ci, os):
                d = _("Origin header protocol does not match this host.")
                raise exception.ValidationError(detail=d)

        si = copy.copy(ci)
        si.token = '***'
        self.msg(_('connect info: %s'), si)

        h = ci.host
        p = ci.port

        self.msg(_("connecting to: %(host)s:%(port)s") % {'host': h, 'port': p})
        t = self.socket(h, p, connect=True)

        if 'internal_access_path' in ci:
            ip = ci.internal_access_path
            if ip:
                t.send(encodeutils.safe_encode(
                    'CONNECT %s HTTP/1.1\r\n\r\n' % ip))
                et = "\r\n\r\n"
                while True:
                    d = t.recv(4096, socket.MSG_PEEK)
                    tl = d.find(et)
                    if tl != -1:
                        if d.split("\r\n")[0].find("200") == -1:
                            raise exception.InvalidConnectionInfo()
                        t.recv(tl + len(et))
                        break

        if self.server.security_proxy is not None:
            ts = TenantSock(self)

            try:
                t = self.server.security_proxy.connect(ts, t)
            except exception.SecurityProxyNegotiationFailed:
                LOG.exception("Unable to perform security proxying, shutting "
                              "down connection")
                ts.close()
                t.shutdown(socket.SHUT_RDWR)
                t.close()
                raise

            ts.finish_up()

        try:
            self.do_proxy(t)
        except Exception:
            if t:
                t.shutdown(socket.SHUT_RDWR)
                t.close()
                self.vmsg(_("%(host)s:%(port)s: "
                          "Websocket client or target closed") %
                          {'host': h, 'port': p})
            raise

    def socket(self, *args, **kwargs):
        return websockifyserver.WebSockifyServer.socket(*args, **kwargs)

    def send_head(self):
        p = self.translate_path(self.path)
        if os.path.isdir(p):
            ps = urlparse.urlsplit(self.path)
            if not ps.path.endswith('/'):
                np = (ps[0], ps[1], ps[2] + '/',
                      ps[3], ps[4])
                nu = urlparse.urlunsplit(np)

                if nu.startswith('//'):
                    self.send_error(HTTPStatus.BAD_REQUEST,
                                    "URI must not start with //")
                    return None

        return super(NovaProxyRequestHandler, self).send_head()


class NovaWebSocketProxy(websockify.WebSocketProxy):
    def __init__(self, *args, **kwargs):
        self.security_proxy = kwargs.pop('security_proxy', None)

        ssl_min_version = kwargs.pop('ssl_minimum_version', None)
        if ssl_min_version and ssl_min_version != 'default':
            kwargs['ssl_options'] = websockify.websocketproxy. \
                                    select_ssl_version(ssl_min_version)

        super(NovaWebSocketProxy, self).__init__(*args, **kwargs)

    @staticmethod
    def get_logger():
        return LOG