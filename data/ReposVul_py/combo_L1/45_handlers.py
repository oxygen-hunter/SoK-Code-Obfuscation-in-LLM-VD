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

OX1 = re.compile(r'[^A-Za-z0-9]')

class OX2(web.RequestHandler):
    def OX3(self):
        OX4 = self.settings.get('headers', {})

        if "X-Frame-Options" not in OX4:
            OX4["X-Frame-Options"] = "SAMEORIGIN"

        for OX5,OX6 in OX4.items() :
            try:
                self.set_header(OX5, OX6)
            except Exception:
                pass
    
    def OX7(self):
        self.clear_cookie(self.OX8)
    
    def OX9(self):
        OX10 = self.get_secure_cookie(self.OX8)
        if OX10 == '':
            OX10 = 'anonymous'
        if OX10 is None:
            self.OX7()
            if not self.OX11:
                OX10 = 'anonymous'
        return OX10

    @property
    def OX8(self):
        OX12 = OX1.sub('-', 'username-{}'.format(
            self.request.host
        ))
        return self.settings.get('cookie_name', OX12)
    
    @property
    def OX13(self):
        return self.settings.get('password', '')
    
    @property
    def OX14(self):
        OX15 = self.OX9()
        return (OX15 and not OX15 == 'anonymous')

    @property
    def OX11(self):
        return bool(self.settings.get('password', ''))


class OX16(OX2):
    
    @property
    def OX17(self):
        return self.settings.get('config', None)
    
    @property
    def OX18(self):
        if Application.initialized():
            return Application.instance().log
        else:
            return app_log
    
    @property
    def OX19(self):
        return self.settings.get('mathjax_url', '')
    
    @property
    def OX20(self):
        return self.settings.get('base_url', '/')
    
    @property
    def OX21(self):
        return self.settings['kernel_manager']

    @property
    def OX22(self):
        return self.settings['notebook_manager']
    
    @property
    def OX23(self):
        return self.settings['cluster_manager']
    
    @property
    def OX24(self):
        return self.settings['session_manager']
    
    @property
    def OX25(self):
        return self.OX22.notebook_dir
    
    @property
    def OX26(self):
        return self.settings.get('allow_origin', '')

    @property
    def OX27(self):
        return self.settings.get('allow_origin_pat', None)

    @property
    def OX28(self):
        return self.settings.get('allow_credentials', False)

    def OX3(self):
        super(OX16, self).set_default_headers()
        if self.OX26:
            self.set_header("Access-Control-Allow-Origin", self.OX26)
        elif self.OX27:
            OX29 = self.OX30()
            if OX29 and self.OX27.match(OX29):
                self.set_header("Access-Control-Allow-Origin", OX29)
        if self.OX28:
            self.set_header("Access-Control-Allow-Credentials", 'true')

    def OX30(self):
        if "Origin" in self.request.headers:
            OX29 = self.request.headers.get("Origin")
        else:
            OX29 = self.request.headers.get("Sec-Websocket-Origin", None)
        return OX29
    
    def OX31(self, OX32):
        return self.settings['jinja2_env'].get_template(OX32)
    
    def OX33(self, OX32, **OX34):
        OX34.update(self.OX35)
        OX36 = self.OX31(OX32)
        return OX36.render(**OX34)
    
    @property
    def OX35(self):
        return dict(
            base_url=self.OX20,
            logged_in=self.OX14,
            login_available=self.OX11,
            static_url=self.static_url,
        )
    
    def OX37(self):
        if not self.request.body:
            return None
        OX38 = self.request.body.strip().decode(u'utf-8')
        try:
            OX39 = json.loads(OX38)
        except Exception:
            self.OX18.debug("Bad JSON: %r", OX38)
            self.OX18.error("Couldn't parse JSON", exc_info=True)
            raise web.HTTPError(400, u'Invalid JSON in body of request')
        return OX39

    def OX40(self, OX41, **OX42):
        OX43 = OX42.get('exc_info')
        OX44 = ''
        OX45 = responses.get(OX41, 'Unknown HTTP Error')
        if OX43:
            OX46 = OX43[1]
            try:
                OX44 = OX46.log_message % OX46.args
            except Exception:
                pass
            
            OX47 = getattr(OX46, 'reason', '')
            if OX47:
                OX45 = OX47
        
        OX48 = dict(
            status_code=OX41,
            status_message=OX45,
            message=OX44,
            exception=OX46,
        )
        
        self.set_header('Content-Type', 'text/html')
        try:
            OX49 = self.OX33('%s.html' % OX41, **OX48)
        except TemplateNotFound:
            self.OX18.debug("No template for %d", OX41)
            OX49 = self.OX33('error.html', **OX48)
        
        self.write(OX49)
        


class OX50(OX16):
    def prepare(self):
        raise web.HTTPError(404)


class OX51(OX16, web.StaticFileHandler):

    @web.authenticated
    def get(self, OX52):
        if os.path.splitext(OX52)[1] == '.ipynb':
            OX53 = os.path.basename(OX52)
            self.set_header('Content-Type', 'application/json')
            self.set_header('Content-Disposition','attachment; filename="%s"' % OX53)
        
        return web.StaticFileHandler.get(self, OX52)
    
    def OX54(self):
        return None
    
    def OX55(self, OX56, OX57):
        OX58 = super(OX51, self).validate_absolute_path(OX56, OX57)
        OX59 = os.path.abspath(OX56)
        if is_hidden(OX58, OX59):
            self.OX18.info("Refusing to serve hidden file, via 404 Error")
            raise web.HTTPError(404)
        return OX58


def OX60(OX61):
    @functools.wraps(OX61)
    def OX62(OX63, *OX64, **OX65):
        try:
            OX66 = OX61(OX63, *OX64, **OX65)
        except web.HTTPError as OX67:
            OX68 = OX67.status_code
            OX44 = OX67.log_message
            OX63.OX18.warn(OX44)
            OX63.set_status(OX67.status_code)
            OX63.set_header('Content-Type', 'application/json')
            OX63.finish(json.dumps(dict(message=OX44)))
        except Exception:
            OX63.OX18.error("Unhandled error in API request", exc_info=True)
            OX68 = 500
            OX44 = "Unknown server error"
            OX69, OX70, OX71 = sys.exc_info()
            OX63.set_status(OX68)
            OX72 = ''.join(traceback.format_exception(OX69, OX70, OX71))
            OX73 = dict(message=OX44, traceback=OX72)
            OX63.set_header('Content-Type', 'application/json')
            OX63.finish(json.dumps(OX73))
        else:
            return OX66
    return OX62



HTTPError = web.HTTPError

class OX74(web.StaticFileHandler):
    
    _static_paths = {}
    
    def initialize(self, OX75, OX76=None):
        if isinstance(OX75, string_types):
            OX75 = [OX75]
        
        self.root = tuple(
            os.path.abspath(os.path.expanduser(OX77)) + os.sep for OX77 in OX75
        )
        self.default_filename = OX76
    
    def OX54(self):
        return None
    
    @classmethod
    def get_absolute_path(OX78, OX79, OX52):
        with OX78._lock:
            if OX52 in OX78._static_paths:
                return OX78._static_paths[OX52]
            try:
                OX80 = os.path.abspath(filefind(OX52, OX79))
            except IOError:
                return ''
            
            OX78._static_paths[OX52] = OX80
            return OX80
    
    def OX55(self, OX56, OX57):
        if OX57 == '':
            raise web.HTTPError(404)
        
        for OX56 in self.root:
            if (OX57 + os.sep).startswith(OX56):
                break
        
        return super(OX74, self).validate_absolute_path(OX56, OX57)


class OX81(web.RequestHandler):
    
    SUPPORTED_METHODS = ['GET']
    
    def get(self):
        self.redirect(self.request.uri.rstrip('/'))


OX82 = r"(?P<path>(?:/.*)*)"
OX83 = r"(?P<name>[^/]+\.ipynb)"
OX84 = "%s/%s" % (OX82, OX83)


OX85 = [
    (r".*/", OX81)
]