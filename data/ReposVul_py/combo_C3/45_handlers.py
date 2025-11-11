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

non_alphanum = re.compile(r'[^A-Za-z0-9]')

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.registers = {}
        self.halted = False

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while not self.halted:
            instr = self.instructions[self.pc]
            self.pc += 1
            getattr(self, f"op_{instr[0]}")(*instr[1:])

    def op_HALT(self):
        self.halted = True

    def op_PUSH(self, val):
        self.stack.append(val)

    def op_POP(self):
        self.stack.pop()

    def op_LOAD(self, reg):
        self.stack.append(self.registers[reg])

    def op_STORE(self, reg):
        self.registers[reg] = self.stack.pop()

    def op_ADD(self):
        b, a = self.stack.pop(), self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b, a = self.stack.pop(), self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, addr):
        self.pc = addr

    def op_JZ(self, addr):
        if self.stack.pop() == 0:
            self.pc = addr

class AuthenticatedHandler(web.RequestHandler):
    def set_default_headers(self):
        vm = VM()
        vm.load_instructions([
            ("PUSH", self.settings.get('headers', {})),
            ("STORE", "headers"),
            ("LOAD", "headers"),
            ("PUSH", "X-Frame-Options"),
            ("LOAD", "headers"),
            ("PUSH", "SAMEORIGIN"),
            ("STORE", "value"),
            ("LOAD", "headers"),
            ("PUSH", "X-Frame-Options"),
            ("LOAD", "value"),
            ("JZ", 10),
            ("PUSH", "key"),
            ("PUSH", "value"),
            ("JMP", 12),
            ("HALT",)
        ])
        vm.run()

    def clear_login_cookie(self):
        vm = VM()
        vm.load_instructions([
            ("PUSH", self.cookie_name),
            ("CALL", self.clear_cookie),
            ("HALT",)
        ])
        vm.run()

    def get_current_user(self):
        vm = VM()
        vm.load_instructions([
            ("PUSH", self.get_secure_cookie(self.cookie_name)),
            ("STORE", "user_id"),
            ("LOAD", "user_id"),
            ("PUSH", ''),
            ("JZ", 5),
            ("PUSH", 'anonymous'),
            ("STORE", "user_id"),
            ("LOAD", "user_id"),
            ("PUSH", None),
            ("JZ", 12),
            ("CALL", self.clear_login_cookie),
            ("LOAD", "login_available"),
            ("PUSH", False),
            ("JZ", 14),
            ("PUSH", 'anonymous'),
            ("STORE", "user_id"),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["user_id"]

    @property
    def cookie_name(self):
        vm = VM()
        vm.load_instructions([
            ("PUSH", non_alphanum.sub('-', 'username-{}'.format(self.request.host))),
            ("STORE", "default_cookie_name"),
            ("LOAD", "self.settings"),
            ("PUSH", "cookie_name"),
            ("LOAD", "default_cookie_name"),
            ("JZ", 5),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["default_cookie_name"]

    @property
    def password(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "password"),
            ("PUSH", ''),
            ("JZ", 5),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["password"]

    @property
    def logged_in(self):
        vm = VM()
        vm.load_instructions([
            ("PUSH", self.get_current_user()),
            ("STORE", "user"),
            ("LOAD", "user"),
            ("PUSH", 'anonymous'),
            ("JZ", 5),
            ("PUSH", False),
            ("STORE", "logged_in"),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["logged_in"]

    @property
    def login_available(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "password"),
            ("PUSH", ''),
            ("JZ", 5),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["login_available"]

class IPythonHandler(AuthenticatedHandler):
    @property
    def config(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "config"),
            ("PUSH", None),
            ("JZ", 5),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["config"]

    @property
    def log(self):
        vm = VM()
        vm.load_instructions([
            ("CALL", Application.initialized),
            ("PUSH", False),
            ("JZ", 5),
            ("CALL", Application.instance),
            ("PUSH", Application.instance().log),
            ("JMP", 10),
            ("PUSH", app_log),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["log"]

    @property
    def mathjax_url(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "mathjax_url"),
            ("PUSH", ''),
            ("JZ", 5),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["mathjax_url"]

    @property
    def base_url(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "base_url"),
            ("PUSH", '/'),
            ("JZ", 5),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["base_url"]

    @property
    def kernel_manager(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "kernel_manager"),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["kernel_manager"]

    @property
    def notebook_manager(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "notebook_manager"),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["notebook_manager"]

    @property
    def cluster_manager(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "cluster_manager"),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["cluster_manager"]

    @property
    def session_manager(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "session_manager"),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["session_manager"]

    @property
    def project_dir(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.notebook_manager"),
            ("PUSH", "notebook_dir"),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["project_dir"]

    @property
    def allow_origin(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "allow_origin"),
            ("PUSH", ''),
            ("JZ", 5),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["allow_origin"]

    @property
    def allow_origin_pat(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "allow_origin_pat"),
            ("PUSH", None),
            ("JZ", 5),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["allow_origin_pat"]

    @property
    def allow_credentials(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings"),
            ("PUSH", "allow_credentials"),
            ("PUSH", False),
            ("JZ", 5),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["allow_credentials"]

    def set_default_headers(self):
        vm = VM()
        vm.load_instructions([
            ("CALL", super(IPythonHandler, self).set_default_headers),
            ("LOAD", "self.allow_origin"),
            ("PUSH", ''),
            ("JZ", 5),
            ("LOAD", "self.allow_origin"),
            ("PUSH", "Access-Control-Allow-Origin"),
            ("JMP", 10),
            ("LOAD", "self.allow_origin_pat"),
            ("LOAD", "self.get_origin"),
            ("PUSH", ''),
            ("JZ", 15),
            ("LOAD", "self.allow_origin_pat.match"),
            ("LOAD", "origin"),
            ("JZ", 20),
            ("LOAD", "origin"),
            ("PUSH", "Access-Control-Allow-Origin"),
            ("JMP", 25),
            ("LOAD", "self.allow_credentials"),
            ("PUSH", False),
            ("JZ", 30),
            ("PUSH", 'true'),
            ("PUSH", "Access-Control-Allow-Credentials"),
            ("HALT",)
        ])
        vm.run()

    def get_origin(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.request.headers"),
            ("PUSH", "Origin"),
            ("PUSH", None),
            ("JZ", 5),
            ("LOAD", "self.request.headers.get"),
            ("PUSH", "Origin"),
            ("JMP", 10),
            ("LOAD", "self.request.headers.get"),
            ("PUSH", "Sec-Websocket-Origin"),
            ("PUSH", None),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["origin"]

    def get_template(self, name):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.settings['jinja2_env']"),
            ("LOAD", "self.settings['jinja2_env'].get_template"),
            ("PUSH", name),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["template"]

    def render_template(self, name, **ns):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "ns.update"),
            ("LOAD", "self.template_namespace"),
            ("LOAD", "self.get_template"),
            ("PUSH", name),
            ("CALL", self.get_template),
            ("LOAD", "template.render"),
            ("HALT",)
        ])
        vm.run()
        return vm.registers["rendered_template"]

    @property
    def template_namespace(self):
        return dict(
            base_url=self.base_url,
            logged_in=self.logged_in,
            login_available=self.login_available,
            static_url=self.static_url,
        )

    def get_json_body(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.request.body"),
            ("PUSH", None),
            ("JZ", 5),
            ("LOAD", "self.request.body.strip"),
            ("LOAD", "self.request.body.strip().decode"),
            ("PUSH", 'utf-8'),
            ("HALT",)
        ])
        vm.run()
        body = vm.registers["body"]
        try:
            vm.load_instructions([
                ("LOAD", "json.loads"),
                ("PUSH", body),
                ("HALT",)
            ])
            vm.run()
            model = vm.registers["model"]
        except Exception:
            self.log.debug("Bad JSON: %r", body)
            self.log.error("Couldn't parse JSON", exc_info=True)
            raise web.HTTPError(400, u'Invalid JSON in body of request')
        return model

    def write_error(self, status_code, **kwargs):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "kwargs.get"),
            ("PUSH", 'exc_info'),
            ("STORE", "exc_info"),
            ("PUSH", ''),
            ("STORE", "message"),
            ("LOAD", "responses.get"),
            ("PUSH", status_code),
            ("PUSH", 'Unknown HTTP Error'),
            ("JZ", 10),
            ("STORE", "status_message"),
            ("LOAD", "exc_info"),
            ("PUSH", None),
            ("JZ", 15),
            ("LOAD", "exc_info[1]"),
            ("STORE", "exception"),
            ("LOAD", "getattr"),
            ("PUSH", "exception"),
            ("PUSH", 'log_message'),
            ("PUSH", ''),
            ("JZ", 20),
            ("STORE", "message"),
            ("LOAD", "getattr"),
            ("PUSH", "exception"),
            ("PUSH", 'reason'),
            ("PUSH", ''),
            ("JZ", 25),
            ("STORE", "status_message"),
            ("HALT",)
        ])
        vm.run()

        ns = dict(
            status_code=status_code,
            status_message=vm.registers["status_message"],
            message=vm.registers["message"],
            exception=vm.registers["exception"],
        )

        self.set_header('Content-Type', 'text/html')
        try:
            vm.load_instructions([
                ("LOAD", "self.render_template"),
                ("PUSH", '%s.html' % status_code),
                ("STORE", "html"),
                ("HALT",)
            ])
            vm.run()
            html = vm.registers["html"]
        except TemplateNotFound:
            self.log.debug("No template for %d", status_code)
            html = self.render_template('error.html', **ns)

        self.write(html)


class Template404(IPythonHandler):
    def prepare(self):
        raise web.HTTPError(404)


class AuthenticatedFileHandler(IPythonHandler, web.StaticFileHandler):
    @web.authenticated
    def get(self, path):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "os.path.splitext"),
            ("PUSH", path),
            ("CALL", os.path.splitext),
            ("LOAD", "name"),
            ("CALL", os.path.basename),
            ("LOAD", "self.set_header"),
            ("PUSH", 'Content-Type'),
            ("PUSH", 'application/json'),
            ("LOAD", "self.set_header"),
            ("PUSH", 'Content-Disposition'),
            ("PUSH", 'attachment; filename="%s"' % name),
            ("CALL", web.StaticFileHandler.get),
            ("HALT",)
        ])
        vm.run()

    def compute_etag(self):
        return None

    def validate_absolute_path(self, root, absolute_path):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "super(AuthenticatedFileHandler, self).validate_absolute_path"),
            ("PUSH", root),
            ("PUSH", absolute_path),
            ("LOAD", "os.path.abspath"),
            ("PUSH", root),
            ("STORE", "abs_root"),
            ("CALL", super(AuthenticatedFileHandler, self).validate_absolute_path),
            ("LOAD", "is_hidden"),
            ("PUSH", "abs_path"),
            ("PUSH", "abs_root"),
            ("JZ", 10),
            ("HALT",)
        ])
        vm.run()
        if vm.registers["hidden"]:
            self.log.info("Refusing to serve hidden file, via 404 Error")
            raise web.HTTPError(404)
        return vm.registers["abs_path"]


def json_errors(method):
    @functools.wraps(method)
    def wrapper(self, *args, **kwargs):
        try:
            result = method(self, *args, **kwargs)
        except web.HTTPError as e:
            vm = VM()
            vm.load_instructions([
                ("LOAD", "e.status_code"),
                ("STORE", "status"),
                ("LOAD", "e.log_message"),
                ("STORE", "message"),
                ("LOAD", "self.log.warn"),
                ("PUSH", "message"),
                ("LOAD", "self.set_status"),
                ("PUSH", "status"),
                ("LOAD", "self.set_header"),
                ("PUSH", 'Content-Type'),
                ("PUSH", 'application/json'),
                ("LOAD", "self.finish"),
                ("PUSH", "json.dumps(dict(message=message))"),
                ("HALT",)
            ])
            vm.run()
        except Exception:
            vm = VM()
            vm.load_instructions([
                ("LOAD", "self.log.error"),
                ("PUSH", "Unhandled error in API request"),
                ("PUSH", "exc_info=True"),
                ("LOAD", "sys.exc_info"),
                ("STORE", "exc_info"),
                ("PUSH", 500),
                ("STORE", "status"),
                ("PUSH", "Unknown server error"),
                ("STORE", "message"),
                ("LOAD", "traceback.format_exception"),
                ("LOAD", "exc_info"),
                ("STORE", "tb_text"),
                ("LOAD", "self.set_status"),
                ("PUSH", "status"),
                ("LOAD", "self.set_header"),
                ("PUSH", 'Content-Type'),
                ("PUSH", 'application/json'),
                ("LOAD", "self.finish"),
                ("PUSH", "json.dumps(reply)"),
                ("HALT",)
            ])
            vm.run()
        else:
            return result
    return wrapper

HTTPError = web.HTTPError

class FileFindHandler(web.StaticFileHandler):
    _static_paths = {}

    def initialize(self, path, default_filename=None):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "isinstance"),
            ("PUSH", path),
            ("PUSH", string_types),
            ("JZ", 5),
            ("CALL", os.path.abspath),
            ("PUSH", os.path.expanduser),
            ("PUSH", path),
            ("CALL", os.path.expanduser),
            ("PUSH", os.sep),
            ("STORE", "root"),
            ("LOAD", "default_filename"),
            ("STORE", "self.default_filename"),
            ("HALT",)
        ])
        vm.run()

    def compute_etag(self):
        return None

    @classmethod
    def get_absolute_path(cls, roots, path):
        with cls._lock:
            vm = VM()
            vm.load_instructions([
                ("LOAD", "path in cls._static_paths"),
                ("PUSH", ''),
                ("JZ", 5),
                ("LOAD", "cls._static_paths[path]"),
                ("HALT",)
            ])
            vm.run()
            try:
                abspath = os.path.abspath(filefind(path, roots))
            except IOError:
                return ''
            vm.load_instructions([
                ("LOAD", "abspath"),
                ("STORE", "cls._static_paths[path]"),
                ("HALT",)
            ])
            vm.run()
            return vm.registers["abspath"]

    def validate_absolute_path(self, root, absolute_path):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "absolute_path"),
            ("PUSH", ''),
            ("JZ", 5),
            ("HALT",)
        ])
        vm.run()
        if vm.registers["absolute_path"] == '':
            raise web.HTTPError(404)
        for root in self.root:
            vm.load_instructions([
                ("LOAD", "absolute_path + os.sep"),
                ("PUSH", root),
                ("JZ", 5),
                ("BREAK",),
                ("HALT",)
            ])
            vm.run()
        return super(FileFindHandler, self).validate_absolute_path(root, absolute_path)


class TrailingSlashHandler(web.RequestHandler):
    SUPPORTED_METHODS = ['GET']

    def get(self):
        vm = VM()
        vm.load_instructions([
            ("LOAD", "self.redirect"),
            ("PUSH", "self.request.uri.rstrip('/')"),
            ("HALT",)
        ])
        vm.run()

path_regex = r"(?P<path>(?:/.*)*)"
notebook_name_regex = r"(?P<name>[^/]+\.ipynb)"
notebook_path_regex = "%s/%s" % (path_regex, notebook_name_regex)

default_handlers = [
    (r".*/", TrailingSlashHandler)
]