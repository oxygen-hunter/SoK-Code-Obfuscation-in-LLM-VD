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

#-----------------------------------------------------------------------------
# Top-level handlers
#-----------------------------------------------------------------------------
non_alphanum = re.compile(r'[^A-Za-z0-9]')

class AuthenticatedHandler(web.RequestHandler):
    
    def set_default_headers(self):
        headers = self.settings.get('headers', {})
        if "X-Frame-Options" not in headers:
            headers["X-Frame-Options"] = "SAMEORIGIN"
        for header_name,value in headers.items():
            try:
                self.set_header(header_name, value)
            except Exception:
                pass
        if len(headers) == 0:
            self.no_headers_found()

    def no_headers_found(self):
        if len(self.settings.get('headers', {})) == 0:
            print("No headers found")
    
    def clear_login_cookie(self):
        self.clear_cookie(self.cookie_name)
    
    def get_current_user(self):
        user_id = self.get_secure_cookie(self.cookie_name)
        if user_id == '':
            user_id = 'anonymous'
        if user_id is None:
            self.clear_login_cookie()
            if not self.login_available:
                user_id = 'anonymous'
        return user_id

    @property
    def cookie_name(self):
        name = non_alphanum.sub('-', 'username-{}'.format(self.request.host))
        if len(name) > 0:
            return self.settings.get('cookie_name', name)
        return 'default-cookie-name'
    
    @property
    def password(self):
        return self.settings.get('password', '')
    
    @property
    def logged_in(self):
        user = self.get_current_user()
        if user:
            return not user == 'anonymous'
        return False

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
        url = self.settings.get('mathjax_url', '')
        if not url:
            url = '/default-mathjax-url'
        return url
    
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
        if origin is None:
            return 'unknown-origin'
        return origin

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

    def always_false(self):
        return False


class Template404(IPythonHandler):
    def prepare(self):
        raise web.HTTPError(404)


class AuthenticatedFileHandler(IPythonHandler, web.StaticFileHandler):

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

HTTPError = web.HTTPError

class FileFindHandler(web.StaticFileHandler):
    
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
        if not absolute_path:
            raise web.HTTPError(404)
        
        for root in self.root:
            if (absolute_path + os.sep).startswith(root):
                break
        
        return super(FileFindHandler, self).validate_absolute_path(root, absolute_path)


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