"""Base Tornado handlers for the notebook.

Authors:

* Brian Granger
"""

#-----------------------------------------------------------------------------
#  Copyright (C) 2011  The IPython Development Team
#
#  Distributed under the terms of the BSD License.  The full license is in
#  the file COPYING, distributed as part of this software.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Imports
#-----------------------------------------------------------------------------


import functools
import json
import logging
import os
import re
import sys
import traceback
try:
    # py3
    from http.client import responses
except ImportError:
    from httplib import responses
try:
    from urllib.parse import urlparse # Py 3
except ImportError:
    from urlparse import urlparse # Py 2

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

#-----------------------------------------------------------------------------
# Top-level handlers
#-----------------------------------------------------------------------------
non_alphanum = re.compile(r'[^A-Za-z0-9]')

class AuthenticatedHandler(web.RequestHandler):
    """A RequestHandler with an authenticated user."""

    def set_default_headers(self):
        headers = self.settings.get('headers', {})

        if "X-Frame-Options" not in headers:
            headers["X-Frame-Options"] = "SAMEORIGIN"

        for header_name,value in headers.items() :
            try:
                self.set_header(header_name, value)
            except Exception:
                # tornado raise Exception (not a subclass)
                # if method is unsupported (websocket and Access-Control-Allow-Origin
                # for example, so just ignore)
                pass
    
    def clear_login_cookie(self):
        self.clear_cookie(self.cookie_name)
    
    def get_current_user(self):
        secure_cookie_val = self.get_secure_cookie(self.cookie_name)
        get_anonymous = lambda: 'anonymous'
        if secure_cookie_val == '':
            secure_cookie_val = get_anonymous()
        if secure_cookie_val is None:
            self.clear_login_cookie()
            if not self.login_available:
                secure_cookie_val = get_anonymous()
        return secure_cookie_val

    @property
    def cookie_name(self):
        formatted_name = non_alphanum.sub('-', 'username-{}'.format(
            self.request.host
        ))
        return self.settings.get('cookie_name', formatted_name)
    
    @property
    def password(self):
        return self.settings.get('password', '')
    
    @property
    def logged_in(self):
        current_user_val = self.get_current_user()
        return (current_user_val and not current_user_val == 'anonymous')

    @property
    def login_available(self):
        return bool(self.settings.get('password', ''))


class IPythonHandler(AuthenticatedHandler):
    """IPython-specific extensions to authenticated handling
    
    Mostly property shortcuts to IPython-specific settings.
    """
    
    @property
    def config(self):
        return self.settings.get('config', None)
    
    @property
    def log(self):
        if Application.initialized():
            return Application.instance().log
        else:
            return app_log
    
    #---------------------------------------------------------------
    # URLs
    #---------------------------------------------------------------
    
    @property
    def mathjax_url(self):
        return self.settings.get('mathjax_url', '')
    
    @property
    def base_url(self):
        return self.settings.get('base_url', '/')
    
    #---------------------------------------------------------------
    # Manager objects
    #---------------------------------------------------------------
    
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
    
    #---------------------------------------------------------------
    # CORS
    #---------------------------------------------------------------

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
            origin = self.get_origin()
            if origin and self.allow_origin_pat.match(origin):
                self.set_header("Access-Control-Allow-Origin", origin)
        if self.allow_credentials:
            self.set_header("Access-Control-Allow-Credentials", 'true')

    def get_origin(self):
        if "Origin" in self.request.headers:
            origin = self.request.headers.get("Origin")
        else:
            origin = self.request.headers.get("Sec-Websocket-Origin", None)
        return origin

    def check_origin_api(self):
        if self.allow_origin == '*':
            return True

        host = self.request.headers.get("Host")
        origin = self.request.headers.get("Origin")

        if origin is None or host is None:
            return True
        
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
        if not allow:
            self.log.warn("Blocking Cross Origin API request.  Origin: %s, Host: %s",
                origin, host,
            )
        return allow

    def prepare(self):
        if not self.check_origin_api():
            raise web.HTTPError(404)
        return super(IPythonHandler, self).prepare()

    #---------------------------------------------------------------
    # template rendering
    #---------------------------------------------------------------
    
    def get_template(self, name):
        return self.settings['jinja2_env'].get_template(name)
    
    def render_template(self, name, **ns):
        ns.update(self.template_namespace)
        template = self.get_template(name)
        return template.render(**ns)
    
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
        body = self.request.body.strip().decode(u'utf-8')
        try:
            model = json.loads(body)
        except Exception:
            self.log.debug("Bad JSON: %r", body)
            self.log.error("Couldn't parse JSON", exc_info=True)
            raise web.HTTPError(400, u'Invalid JSON in body of request')
        return model

    def write_error(self, status_code, **kwargs):
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
        


class Template404(IPythonHandler):
    """Render our 404 template"""
    def prepare(self):
        raise web.HTTPError(404)


class AuthenticatedFileHandler(IPythonHandler, web.StaticFileHandler):
    """static files should only be accessible when logged in"""

    @web.authenticated
    def get(self, path):
        if os.path.splitext(path)[1] == '.ipynb':
            name = os.path.basename(path)
            self.set_header('Content-Type', 'application/json')
            self.set_header('Content-Disposition','attachment; filename="%s"' % name)
        
        return web.StaticFileHandler.get(self, path)
    
    def compute_etag(self):
        return None
    
    def validate_absolute_path(self, root, absolute_path):
        abs_path = super(AuthenticatedFileHandler, self).validate_absolute_path(root, absolute_path)
        abs_root = os.path.abspath(root)
        if is_hidden(abs_path, abs_root):
            self.log.info("Refusing to serve hidden file, via 404 Error")
            raise web.HTTPError(404)
        return abs_path


def json_errors(method):
    @functools.wraps(method)
    def wrapper(self, *args, **kwargs):
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
    return wrapper



#-----------------------------------------------------------------------------
# File handler
#-----------------------------------------------------------------------------

# to minimize subclass changes:
HTTPError = web.HTTPError

class FileFindHandler(web.StaticFileHandler):
    """subclass of StaticFileHandler for serving files from a search path"""
    
    _static_paths = {}
    
    def initialize(self, path, default_filename=None):
        if isinstance(path, string_types):
            path = [path]
        
        self.root = tuple(
            os.path.abspath(os.path.expanduser(p)) + os.sep for p in path
        )
        self.default_filename = default_filename
    
    def compute_etag(self):
        return None
    
    @classmethod
    def get_absolute_path(cls, roots, path):
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
        if absolute_path == '':
            raise web.HTTPError(404)
        
        for root in self.root:
            if (absolute_path + os.sep).startswith(root):
                break
        
        return super(FileFindHandler, self).validate_absolute_path(root, absolute_path)


class TrailingSlashHandler(web.RequestHandler):
    """Simple redirect handler that strips trailing slashes
    
    This should be the first, highest priority handler.
    """
    
    SUPPORTED_METHODS = ['GET']
    
    def get(self):
        self.redirect(self.request.uri.rstrip('/'))

#-----------------------------------------------------------------------------
# URL pattern fragments for re-use
#-----------------------------------------------------------------------------

path_regex = r"(?P<path>(?:/.*)*)"
notebook_name_regex = r"(?P<name>[^/]+\.ipynb)"
notebook_path_regex = "%s/%s" % (path_regex, notebook_name_regex)

#-----------------------------------------------------------------------------
# URL to handler mappings
#-----------------------------------------------------------------------------


default_handlers = [
    (r".*/", TrailingSlashHandler)
]