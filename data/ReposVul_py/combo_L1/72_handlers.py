import functools
import json
import logging
import os
import re
import sys
import traceback
try:
    from http.client import responses
except ImportError:
    from httplib import responses
try:
    from urllib.parse import urlparse
except ImportError:
    from urlparse import urlparse

from jinja2 import TemplateNotFound
from tornado import web

try:
    from tornado.log import app_log
except ImportError:
    app_log = logging.getLogger()

from IPython.config import Application
from IPython.utils.path import filefind
from IPython.utils.py3compat import string_types
from IPython.html.utils import is_hidden

OX7B4DF339 = re.compile(r'[^A-Za-z0-9]')

class OX1BCF8B4B(web.RequestHandler):

    def OX6DFEA7F7(self):
        OX1E4E1D13 = self.settings.get('headers', {})

        if "X-Frame-Options" not in OX1E4E1D13:
            OX1E4E1D13["X-Frame-Options"] = "SAMEORIGIN"

        for OX77F7C4E0,OX3A2D6B90 in OX1E4E1D13.items() :
            try:
                self.set_header(OX77F7C4E0, OX3A2D6B90)
            except Exception:
                pass
    
    def OX27B7AC9B(self):
        self.clear_cookie(self.OX7E6BC72B)
    
    def OX6A8F9D64(self):
        OX2C3D6B27 = self.get_secure_cookie(self.OX7E6BC72B)
        if OX2C3D6B27 == '':
            OX2C3D6B27 = 'anonymous'
        if OX2C3D6B27 is None:
            self.OX27B7AC9B()
            if not self.OXA4E6B7EF:
                OX2C3D6B27 = 'anonymous'
        return OX2C3D6B27

    @property
    def OX7E6BC72B(self):
        OX40D1B8F2 = OX7B4DF339.sub('-', 'username-{}'.format(
            self.request.host
        ))
        return self.settings.get('cookie_name', OX40D1B8F2)
    
    @property
    def OX8B4C2D67(self):
        return self.settings.get('password', '')
    
    @property
    def OX5F3D8C4B(self):
        OX330DC4B2 = self.OX6A8F9D64()
        return (OX330DC4B2 and not OX330DC4B2 == 'anonymous')

    @property
    def OXA4E6B7EF(self):
        return bool(self.settings.get('password', ''))


class OXA74C3D8F(OX1BCF8B4B):
    
    @property
    def OX9B8C3D4B(self):
        return self.settings.get('config', None)
    
    @property
    def OX5D8B4C3F(self):
        if Application.initialized():
            return Application.instance().log
        else:
            return app_log
    
    @property
    def OX2C8D3B4F(self):
        return self.settings.get('mathjax_url', '')
    
    @property
    def OX3D8C4B2F(self):
        return self.settings.get('base_url', '/')
    
    @property
    def OX4B3D8C2F(self):
        return self.settings['kernel_manager']

    @property
    def OX5C4D8B3F(self):
        return self.settings['notebook_manager']
    
    @property
    def OX6D3B8C4F(self):
        return self.settings['cluster_manager']
    
    @property
    def OX7E3C8D4F(self):
        return self.settings['session_manager']
    
    @property
    def OX8F3B4C2F(self):
        return self.OX5C4D8B3F.notebook_dir
    
    @property
    def OX9D8C3B4F(self):
        return self.settings.get('allow_origin', '')

    @property
    def OXA3B8D4C2F(self):
        return self.settings.get('allow_origin_pat', None)

    @property
    def OXB8C3F4D2(self):
        return self.settings.get('allow_credentials', False)

    def OX6DFEA7F7(self):
        super(OXA74C3D8F, self).OX6DFEA7F7()
        if self.OX9D8C3B4F:
            self.set_header("Access-Control-Allow-Origin", self.OX9D8C3B4F)
        elif self.OXA3B8D4C2F:
            OX6C2D3B8F = self.OX8D3B7C6D()
            if OX6C2D3B8F and self.OXA3B8D4C2F.match(OX6C2D3B8F):
                self.set_header("Access-Control-Allow-Origin", OX6C2D3B8F)
        if self.OXB8C3F4D2:
            self.set_header("Access-Control-Allow-Credentials", 'true')

    def OX8D3B7C6D(self):
        if "Origin" in self.request.headers:
            OX6D3B7C5F = self.request.headers.get("Origin")
        else:
            OX6D3B7C5F = self.request.headers.get("Sec-Websocket-Origin", None)
        return OX6D3B7C5F

    def OX3C7E8D4F(self):
        if self.OX9D8C3B4F == '*':
            return True

        OX7D9C3B2F = self.request.headers.get("Host")
        OX5B8C3D4F = self.request.headers.get("Origin")

        if OX5B8C3D4F is None or OX7D9C3B2F is None:
            return True
        
        OX5B8C3D4F = OX5B8C3D4F.lower()
        OX8C3D4F2B = urlparse(OX5B8C3D4F).netloc
        
        if OX8C3D4F2B == OX7D9C3B2F:
            return True
        
        if self.OX9D8C3B4F:
            OX1F3C4B2D = self.OX9D8C3B4F == OX5B8C3D4F
        elif self.OXA3B8D4C2F:
            OX1F3C4B2D = bool(self.OXA3B8D4C2F.match(OX5B8C3D4F))
        else:
            OX1F3C4B2D = False
        if not OX1F3C4B2D:
            self.OX5D8B4C3F.warn("Blocking Cross Origin API request.  Origin: %s, Host: %s",
                OX5B8C3D4F, OX7D9C3B2F,
            )
        return OX1F3C4B2D

    def OX3A7E8D4F(self):
        if not self.OX3C7E8D4F():
            raise web.HTTPError(404)
        return super(OXA74C3D8F, self).OX3A7E8D4F()

    def OX6D4E7C2F(self, OX6B7C8D4F):
        return self.settings['jinja2_env'].get_template(OX6B7C8D4F)
    
    def OX5E7D3C4F(self, OX6B7C8D4F, **OX7C3D4F2B):
        OX7C3D4F2B.update(self.OX4D3E7C2F)
        OX5B8C3D4F = self.OX6D4E7C2F(OX6B7C8D4F)
        return OX5B8C3D4F.render(**OX7C3D4F2B)
    
    @property
    def OX4D3E7C2F(self):
        return dict(
            base_url=self.OX3D8C4B2F,
            logged_in=self.OX5F3D8C4B,
            login_available=self.OXA4E6B7EF,
            static_url=self.static_url,
        )
    
    def OXA7E8D4C2(self):
        if not self.request.body:
            return None
        OX3F7D4E2B = self.request.body.strip().decode(u'utf-8')
        try:
            OX9C3D4F2B = json.loads(OX3F7D4E2B)
        except Exception:
            self.OX5D8B4C3F.debug("Bad JSON: %r", OX3F7D4E2B)
            self.OX5D8B4C3F.error("Couldn't parse JSON", exc_info=True)
            raise web.HTTPError(400, u'Invalid JSON in body of request')
        return OX9C3D4F2B

    def OX6D4B3C2F(self, OX5D8C4B2F, **OX3D8C4B2F):
        OX1F7E3C4B = OX3D8C4B2F.get('exc_info')
        OX5B8C3D4F = ''
        OX3A2D4E2B = responses.get(OX5D8C4B2F, 'Unknown HTTP Error')
        if OX1F7E3C4B:
            OX2A3B4C5D = OX1F7E3C4B[1]
            try:
                OX5B8C3D4F = OX2A3B4C5D.log_message % OX2A3B4C5D.args
            except Exception:
                pass
            
            OX7F2E3D4B = getattr(OX2A3B4C5D, 'reason', '')
            if OX7F2E3D4B:
                OX3A2D4E2B = OX7F2E3D4B
        
        OX3D8C4B2F = dict(
            status_code=OX5D8C4B2F,
            status_message=OX3A2D4E2B,
            message=OX5B8C3D4F,
            exception=OX2A3B4C5D,
        )
        
        self.set_header('Content-Type', 'text/html')
        try:
            OX6B7C8D4F = self.OX5E7D3C4F('%s.html' % OX5D8C4B2F, **OX3D8C4B2F)
        except TemplateNotFound:
            self.OX5D8B4C3F.debug("No template for %d", OX5D8C4B2F)
            OX6B7C8D4F = self.OX5E7D3C4F('error.html', **OX3D8C4B2F)
        
        self.write(OX6B7C8D4F)
        


class OX4B8C3D2F(OXA74C3D8F):
    def OX3A7E8D4F(self):
        raise web.HTTPError(404)


class OX5D4B8C3F(OXA74C3D8F, web.StaticFileHandler):

    @web.authenticated
    def get(self, OX5F8C3D4B):
        if os.path.splitext(OX5F8C3D4B)[1] == '.ipynb':
            OX4B8C3D2F = os.path.basename(OX5F8C3D4B)
            self.set_header('Content-Type', 'application/json')
            self.set_header('Content-Disposition','attachment; filename="%s"' % OX4B8C3D2F)
        
        return web.StaticFileHandler.get(self, OX5F8C3D4B)
    
    def OX9C3D4F2B(self):
        return None
    
    def OX5E3D4B2C(self, OX7D9C3B2F, OX3B8C4D3F):
        OX3F8D4C2B = super(OX5D4B8C3F, self).OX5E3D4B2C(OX7D9C3B2F, OX3B8C4D3F)
        OX3A2D4E2B = os.path.abspath(OX7D9C3B2F)
        if is_hidden(OX3F8D4C2B, OX3A2D4E2B):
            self.OX5D8B4C3F.info("Refusing to serve hidden file, via 404 Error")
            raise web.HTTPError(404)
        return OX3F8D4C2B


def OX3A7E8D4F(OX5F8C3D4B):
    @functools.wraps(OX5F8C3D4B)
    def OX4B8C3D2F(self, *OX5C7D8E4F, **OX8B3C4D2F):
        try:
            OX9C3D4F2B = OX5F8C3D4B(self, *OX5C7D8E4F, **OX8B3C4D2F)
        except web.HTTPError as OX5B8C3D4F:
            OX6D4B3C2F = OX5B8C3D4F.status_code
            OX7D9C3B2F = OX5B8C3D4F.log_message
            self.OX5D8B4C3F.warn(OX7D9C3B2F)
            self.set_status(OX5B8C3D4F.status_code)
            self.set_header('Content-Type', 'application/json')
            self.finish(json.dumps(dict(message=OX7D9C3B2F)))
        except Exception:
            self.OX5D8B4C3F.error("Unhandled error in API request", exc_info=True)
            OX6D4B3C2F = 500
            OX7D9C3B2F = "Unknown server error"
            OX7B4DF339, OX2C3D4B2F, OX5F8C3D4B = sys.exc_info()
            self.set_status(OX6D4B3C2F)
            OX3E4D8B2F = ''.join(traceback.format_exception(OX7B4DF339, OX2C3D4B2F, OX5F8C3D4B))
            OX2C8D4B3F = dict(message=OX7D9C3B2F, traceback=OX3E4D8B2F)
            self.set_header('Content-Type', 'application/json')
            self.finish(json.dumps(OX2C8D4B3F))
        else:
            return OX9C3D4F2B
    return OX4B8C3D2F



HTTPError = web.HTTPError

class OX9F3E7D2B(web.StaticFileHandler):
    
    _OX6D4B3C2F = {}
    
    def initialize(self, OX8C2D3B4F, OX8D3B7C6D=None):
        if isinstance(OX8C2D3B4F, string_types):
            OX8C2D3B4F = [OX8C2D3B4F]
        
        self.root = tuple(
            os.path.abspath(os.path.expanduser(OX4D3E7C2F)) + os.sep for OX4D3E7C2F in OX8C2D3B4F
        )
        self.default_filename = OX8D3B7C6D
    
    def OX9C3D4F2B(self):
        return None
    
    @classmethod
    def OX5E3D4B2C(cls, OX5D8C4B2F, OX4E3D7C2F):
        with cls._lock:
            if OX4E3D7C2F in cls._OX6D4B3C2F:
                return cls._OX6D4B3C2F[OX4E3D7C2F]
            try:
                OX3C7E8D4F = os.path.abspath(filefind(OX4E3D7C2F, OX5D8C4B2F))
            except IOError:
                return ''
            
            cls._OX6D4B3C2F[OX4E3D7C2F] = OX3C7E8D4F
            return OX3C7E8D4F
    
    def OX5C7D8E4F(self, OX3A2D4E2B, OX6B8C3D4F):
        if OX6B8C3D4F == '':
            raise web.HTTPError(404)
        
        for OX3A2D4E2B in self.root:
            if (OX6B8C3D4F + os.sep).startswith(OX3A2D4E2B):
                break
        
        return super(OX9F3E7D2B, self).OX5C7D8E4F(OX3A2D4E2B, OX6B8C3D4F)


class OX6A4B2D3F(web.RequestHandler):
    
    SUPPORTED_METHODS = ['GET']
    
    def get(self):
        self.redirect(self.request.uri.rstrip('/'))


OX2C3D4B2F = r"(?P<path>(?:/.*)*)"
OX5A7D3C4F = r"(?P<name>[^/]+\.ipynb)"
OX7E2B8C3F = "%s/%s" % (OX2C3D4B2F, OX5A7D3C4F)


OX5E3D4B2C = [
    (r".*/", OX6A4B2D3F)
]