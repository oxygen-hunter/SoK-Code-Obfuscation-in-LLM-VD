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
        dispatcher = 0
        while True:
            if dispatcher == 0:
                headers = self.settings.get('headers', {})
                if "X-Frame-Options" not in headers:
                    headers["X-Frame-Options"] = "SAMEORIGIN"
                dispatcher = 1
            elif dispatcher == 1:
                for header_name,value in headers.items():
                    try:
                        self.set_header(header_name, value)
                    except Exception:
                        pass
                break

    def clear_login_cookie(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.clear_cookie(self.cookie_name)
                break
    
    def get_current_user(self):
        dispatcher = 0
        user_id = None
        while True:
            if dispatcher == 0:
                user_id = self.get_secure_cookie(self.cookie_name)
                if user_id == '':
                    user_id = 'anonymous'
                dispatcher = 1
            elif dispatcher == 1:
                if user_id is None:
                    self.clear_login_cookie()
                    if not self.login_available:
                        user_id = 'anonymous'
                break
        return user_id

    @property
    def cookie_name(self):
        dispatcher = 0
        default_cookie_name = None
        while True:
            if dispatcher == 0:
                default_cookie_name = non_alphanum.sub('-', 'username-{}'.format(self.request.host))
                break
        return self.settings.get('cookie_name', default_cookie_name)
    
    @property
    def password(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings.get('password', '')
    
    @property
    def logged_in(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                user = self.get_current_user()
                return (user and not user == 'anonymous')

    @property
    def login_available(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return bool(self.settings.get('password', ''))

class IPythonHandler(AuthenticatedHandler):
    
    @property
    def config(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings.get('config', None)
    
    @property
    def log(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if Application.initialized():
                    return Application.instance().log
                else:
                    return app_log
    
    @property
    def mathjax_url(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings.get('mathjax_url', '')
    
    @property
    def base_url(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings.get('base_url', '/')
    
    @property
    def kernel_manager(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings['kernel_manager']

    @property
    def notebook_manager(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings['notebook_manager']
    
    @property
    def cluster_manager(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings['cluster_manager']
    
    @property
    def session_manager(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings['session_manager']
    
    @property
    def project_dir(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.notebook_manager.notebook_dir
    
    @property
    def allow_origin(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings.get('allow_origin', '')

    @property
    def allow_origin_pat(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings.get('allow_origin_pat', None)

    @property
    def allow_credentials(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings.get('allow_credentials', False)

    def set_default_headers(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                super(IPythonHandler, self).set_default_headers()
                if self.allow_origin:
                    self.set_header("Access-Control-Allow-Origin", self.allow_origin)
                elif self.allow_origin_pat:
                    origin = self.get_origin()
                    if origin and self.allow_origin_pat.match(origin):
                        self.set_header("Access-Control-Allow-Origin", origin)
                dispatcher = 1
            elif dispatcher == 1:
                if self.allow_credentials:
                    self.set_header("Access-Control-Allow-Credentials", 'true')
                break

    def get_origin(self):
        dispatcher = 0
        origin = None
        while True:
            if dispatcher == 0:
                if "Origin" in self.request.headers:
                    origin = self.request.headers.get("Origin")
                else:
                    origin = self.request.headers.get("Sec-Websocket-Origin", None)
                break
        return origin

    def check_origin_api(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if self.allow_origin == '*':
                    return True

                host = self.request.headers.get("Host")
                origin = self.request.headers.get("Origin")

                if origin is None or host is None:
                    return True
                dispatcher = 1
            elif dispatcher == 1:
                origin = origin.lower()
                origin_host = urlparse(origin).netloc

                if origin_host == host:
                    return True
                
                if self.allow_origin:
                    allow = self.allow_origin == origin
                elif self.allow_origin_pat:
                    allow = bool(self.allow_origin_pat.match(origin))
                else:
                    allow = False
                dispatcher = 2
            elif dispatcher == 2:
                if not allow:
                    self.log.warn("Blocking Cross Origin API request.  Origin: %s, Host: %s", origin, host)
                return allow

    def prepare(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if not self.check_origin_api():
                    raise web.HTTPError(404)
                return super(IPythonHandler, self).prepare()

    def get_template(self, name):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.settings['jinja2_env'].get_template(name)
    
    def render_template(self, name, **ns):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                ns.update(self.template_namespace)
                template = self.get_template(name)
                return template.render(**ns)
    
    @property
    def template_namespace(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return dict(
                    base_url=self.base_url,
                    logged_in=self.logged_in,
                    login_available=self.login_available,
                    static_url=self.static_url,
                )
    
    def get_json_body(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if not self.request.body:
                    return None
                body = self.request.body.strip().decode(u'utf-8')
                dispatcher = 1
            elif dispatcher == 1:
                try:
                    model = json.loads(body)
                except Exception:
                    self.log.debug("Bad JSON: %r", body)
                    self.log.error("Couldn't parse JSON", exc_info=True)
                    raise web.HTTPError(400, u'Invalid JSON in body of request')
                return model

    def write_error(self, status_code, **kwargs):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                exc_info = kwargs.get('exc_info')
                message = ''
                status_message = responses.get(status_code, 'Unknown HTTP Error')
                if exc_info:
                    exception = exc_info[1]
                    try:
                        message = exception.log_message % exception.args
                    except Exception:
                        pass
                    reason = getattr(exception, 'reason', '')
                    if reason:
                        status_message = reason
                dispatcher = 1
            elif dispatcher == 1:
                ns = dict(
                    status_code=status_code,
                    status_message=status_message,
                    message=message,
                    exception=exception,
                )
                self.set_header('Content-Type', 'text/html')
                try:
                    html = self.render_template('%s.html' % status_code, **ns)
                except TemplateNotFound:
                    self.log.debug("No template for %d", status_code)
                    html = self.render_template('error.html', **ns)
                self.write(html)
                break

class Template404(IPythonHandler):

    def prepare(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                raise web.HTTPError(404)

class AuthenticatedFileHandler(IPythonHandler, web.StaticFileHandler):

    @web.authenticated
    def get(self, path):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if os.path.splitext(path)[1] == '.ipynb':
                    name = os.path.basename(path)
                    self.set_header('Content-Type', 'application/json')
                    self.set_header('Content-Disposition','attachment; filename="%s"' % name)
                return web.StaticFileHandler.get(self, path)

    def compute_etag(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return None
    
    def validate_absolute_path(self, root, absolute_path):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                abs_path = super(AuthenticatedFileHandler, self).validate_absolute_path(root, absolute_path)
                abs_root = os.path.abspath(root)
                if is_hidden(abs_path, abs_root):
                    self.log.info("Refusing to serve hidden file, via 404 Error")
                    raise web.HTTPError(404)
                return abs_path

def json_errors(method):

    @functools.wraps(method)
    def wrapper(self, *args, **kwargs):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                try:
                    result = method(self, *args, **kwargs)
                except web.HTTPError as e:
                    status = e.status_code
                    message = e.log_message
                    self.log.warn(message)
                    self.set_status(e.status_code)
                    self.set_header('Content-Type', 'application/json')
                    self.finish(json.dumps(dict(message=message)))
                except Exception:
                    self.log.error("Unhandled error in API request", exc_info=True)
                    status = 500
                    message = "Unknown server error"
                    t, value, tb = sys.exc_info()
                    self.set_status(status)
                    tb_text = ''.join(traceback.format_exception(t, value, tb))
                    reply = dict(message=message, traceback=tb_text)
                    self.set_header('Content-Type', 'application/json')
                    self.finish(json.dumps(reply))
                else:
                    return result
                break
    return wrapper

HTTPError = web.HTTPError

class FileFindHandler(web.StaticFileHandler):
    
    _static_paths = {}
    
    def initialize(self, path, default_filename=None):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if isinstance(path, string_types):
                    path = [path]
                self.root = tuple(os.path.abspath(os.path.expanduser(p)) + os.sep for p in path)
                self.default_filename = default_filename
                break
    
    def compute_etag(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return None
    
    @classmethod
    def get_absolute_path(cls, roots, path):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                with cls._lock:
                    if path in cls._static_paths:
                        return cls._static_paths[path]
                    try:
                        abspath = os.path.abspath(filefind(path, roots))
                    except IOError:
                        return ''
                    cls._static_paths[path] = abspath
                    return abspath

    def validate_absolute_path(self, root, absolute_path):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if absolute_path == '':
                    raise web.HTTPError(404)
                for root in self.root:
                    if (absolute_path + os.sep).startswith(root):
                        break
                return super(FileFindHandler, self).validate_absolute_path(root, absolute_path)

class TrailingSlashHandler(web.RequestHandler):
    
    SUPPORTED_METHODS = ['GET']
    
    def get(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.redirect(self.request.uri.rstrip('/'))
                break

path_regex = r"(?P<path>(?:/.*)*)"
notebook_name_regex = r"(?P<name>[^/]+\.ipynb)"
notebook_path_regex = "%s/%s" % (path_regex, notebook_name_regex)

default_handlers = [
    (r".*/", TrailingSlashHandler)
]