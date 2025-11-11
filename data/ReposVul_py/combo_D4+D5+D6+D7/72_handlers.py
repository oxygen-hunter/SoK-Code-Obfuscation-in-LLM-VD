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

non_alphanum = re.compile(r'[^A-Za-z0-9]')

class AuthenticatedHandler(web.RequestHandler):

    def set_default_headers(self):
        h = self.settings.get('headers', {})
        if "X-Frame-Options" not in h:
            h["X-Frame-Options"] = "SAMEORIGIN"
        for n,v in h.items() :
            try:
                self.set_header(n, v)
            except Exception:
                pass
    
    def clear_login_cookie(self):
        self.clear_cookie(self.cookie_name)
    
    def get_current_user(self):
        u = self.get_secure_cookie(self.cookie_name)
        a = 'anonymous'
        if u == '':
            u = a
        if u is None:
            self.clear_login_cookie()
            if not self.login_available:
                u = a
        return u

    @property
    def cookie_name(self):
        x = non_alphanum.sub('-', 'username-{}'.format(
            self.request.host
        ))
        return self.settings.get('cookie_name', x)
    
    @property
    def password(self):
        return self.settings.get('password', '')
    
    @property
    def logged_in(self):
        z = self.get_current_user()
        return (z and not z == 'anonymous')

    @property
    def login_available(self):
        return bool(self.settings.get('password', ''))

class IPythonHandler(AuthenticatedHandler):

    @property
    def config(self):
        return self.settings.get('config', None)
    
    @property
    def log(self):
        if Application.initialized():
            return Application.instance().log
        else:
            return app_log
    
    @property
    def mathjax_url(self):
        return self.settings.get('mathjax_url', '')
    
    @property
    def base_url(self):
        return self.settings.get('base_url', '/')
    
    @property
    def kernel_manager(self):
        return self.settings['kernel_manager']

    @property
    def notebook_manager(self):
        return self.settings['notebook_manager']
    
    @property
    def cluster_manager(self):
        return self.settings['cluster_manager']
    
    @property
    def session_manager(self):
        return self.settings['session_manager']
    
    @property
    def project_dir(self):
        return self.notebook_manager.notebook_dir
    
    @property
    def allow_origin(self):
        return self.settings.get('allow_origin', '')

    @property
    def allow_origin_pat(self):
        return self.settings.get('allow_origin_pat', None)

    @property
    def allow_credentials(self):
        return self.settings.get('allow_credentials', False)

    def set_default_headers(self):
        super(IPythonHandler, self).set_default_headers()
        if self.allow_origin:
            self.set_header("Access-Control-Allow-Origin", self.allow_origin)
        elif self.allow_origin_pat:
            o = self.get_origin()
            if o and self.allow_origin_pat.match(o):
                self.set_header("Access-Control-Allow-Origin", o)
        if self.allow_credentials:
            self.set_header("Access-Control-Allow-Credentials", 'true')

    def get_origin(self):
        o = self.request.headers.get("Origin", 
            self.request.headers.get("Sec-Websocket-Origin", None))
        return o

    def check_origin_api(self):
        if self.allow_origin == '*':
            return True

        h = self.request.headers.get("Host")
        o = self.request.headers.get("Origin")

        if o is None or h is None:
            return True
        
        o = o.lower()
        o_h = urlparse(o).netloc
        
        if o_h == h:
            return True
        
        if self.allow_origin:
            allow = self.allow_origin == o
        elif self.allow_origin_pat:
            allow = bool(self.allow_origin_pat.match(o))
        else:
            allow = False
        if not allow:
            self.log.warn("Blocking Cross Origin API request.  Origin: %s, Host: %s",
                o, h,
            )
        return allow

    def prepare(self):
        if not self.check_origin_api():
            raise web.HTTPError(404)
        return super(IPythonHandler, self).prepare()
    
    def get_template(self, n):
        return self.settings['jinja2_env'].get_template(n)
    
    def render_template(self, n, **ns):
        ns.update(self.template_namespace)
        t = self.get_template(n)
        return t.render(**ns)
    
    @property
    def template_namespace(self):
        return dict(
            base_url=self.base_url,
            logged_in=self.logged_in,
            login_available=self.login_available,
            static_url=self.static_url,
        )
    
    def get_json_body(self):
        if not self.request.body:
            return None
        b = self.request.body.strip().decode(u'utf-8')
        try:
            m = json.loads(b)
        except Exception:
            self.log.debug("Bad JSON: %r", b)
            self.log.error("Couldn't parse JSON", exc_info=True)
            raise web.HTTPError(400, u'Invalid JSON in body of request')
        return m

    def write_error(self, s_c, **kwargs):
        e_i = kwargs.get('exc_info')
        m = ''
        s_m = responses.get(s_c, 'Unknown HTTP Error')
        if e_i:
            e = e_i[1]
            try:
                m = e.log_message % e.args
            except Exception:
                pass
            
            r = getattr(e, 'reason', '')
            if r:
                s_m = r
        
        n = dict(
            status_code=s_c,
            status_message=s_m,
            message=m,
            exception=e,
        )
        
        self.set_header('Content-Type', 'text/html')
        try:
            h = self.render_template('%s.html' % s_c, **n)
        except TemplateNotFound:
            self.log.debug("No template for %d", s_c)
            h = self.render_template('error.html', **n)
        
        self.write(h)
        
class Template404(IPythonHandler):
    def prepare(self):
        raise web.HTTPError(404)

class AuthenticatedFileHandler(IPythonHandler, web.StaticFileHandler):

    @web.authenticated
    def get(self, p):
        if os.path.splitext(p)[1] == '.ipynb':
            n = os.path.basename(p)
            self.set_header('Content-Type', 'application/json')
            self.set_header('Content-Disposition','attachment; filename="%s"' % n)
        
        return web.StaticFileHandler.get(self, p)
    
    def compute_etag(self):
        return None
    
    def validate_absolute_path(self, root, a_p):
        a_p2 = super(AuthenticatedFileHandler, self).validate_absolute_path(root, a_p)
        a_r = os.path.abspath(root)
        if is_hidden(a_p2, a_r):
            self.log.info("Refusing to serve hidden file, via 404 Error")
            raise web.HTTPError(404)
        return a_p2

def json_errors(m):
    @functools.wraps(m)
    def wrapper(self, *args, **kwargs):
        try:
            r = m(self, *args, **kwargs)
        except web.HTTPError as e:
            s = e.status_code
            m = e.log_message
            self.log.warn(m)
            self.set_status(e.status_code)
            self.set_header('Content-Type', 'application/json')
            self.finish(json.dumps(dict(message=m)))
        except Exception:
            self.log.error("Unhandled error in API request", exc_info=True)
            s = 500
            m = "Unknown server error"
            t, v, tb = sys.exc_info()
            self.set_status(s)
            tb_t = ''.join(traceback.format_exception(t, v, tb))
            r = dict(message=m, traceback=tb_t)
            self.set_header('Content-Type', 'application/json')
            self.finish(json.dumps(r))
        else:
            return r
    return wrapper

HTTPError = web.HTTPError

class FileFindHandler(web.StaticFileHandler):
    
    _s_p = {}
    
    def initialize(self, p, d_f=None):
        if isinstance(p, string_types):
            p = [p]
        
        self.root = tuple(
            os.path.abspath(os.path.expanduser(a_p)) + os.sep for a_p in p
        )
        self.default_filename = d_f
    
    def compute_etag(self):
        return None
    
    @classmethod
    def get_absolute_path(cls, roots, p):
        with cls._lock:
            if p in cls._s_p:
                return cls._s_p[p]
            try:
                a_p = os.path.abspath(filefind(p, roots))
            except IOError:
                return ''
            
            cls._s_p[p] = a_p
            return a_p
    
    def validate_absolute_path(self, root, a_p):
        if a_p == '':
            raise web.HTTPError(404)
        
        for r in self.root:
            if (a_p + os.sep).startswith(r):
                break
        
        return super(FileFindHandler, self).validate_absolute_path(root, a_p)

class TrailingSlashHandler(web.RequestHandler):
    
    SUPPORTED_METHODS = ['GET']
    
    def get(self):
        self.redirect(self.request.uri.rstrip('/'))

path_regex = r"(?P<path>(?:/.*)*)"
notebook_name_regex = r"(?P<name>[^/]+\.ipynb)"
notebook_path_regex = "%s/%s" % (path_regex, notebook_name_regex)

default_handlers = [
    (r".*/", TrailingSlashHandler)
]