# -*- coding: utf-8 -*-
#       ____________
#   ___/       |    \_____________ _                 _ ___
#  /        ___/    |    _ __ _  _| |   ___  __ _ __| |   \
# /    \___/  ______/   | '_ \ || | |__/ _ \/ _` / _` |    \
# \            ◯ |      | .__/\_, |____\___/\__,_\__,_|    /
#  \_______\    /_______|_|   |__/________________________/
#           \  /
#            \/

import os
import flask
import jinja2
from werkzeug.serving import WSGIRequestHandler
from .blueprints import BLUEPRINTS
from .config import get_default_config
from .extensions import EXTENSIONS, THEMES
from .filters import TEMPLATE_FILTERS
from .globals import TEMPLATE_GLOBALS
from .handlers import ERROR_HANDLERS
from .helpers import JSONEncoder
from .processors import CONTEXT_PROCESSORS

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.running and self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.dispatch(instr)

    def dispatch(self, instr):
        op, *args = instr
        if op == 'PUSH':
            self.stack.append(args[0])
        elif op == 'POP':
            self.stack.pop()
        elif op == 'ADD':
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)
        elif op == 'SUB':
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)
        elif op == 'JMP':
            self.pc = args[0]
            return
        elif op == 'JZ':
            if self.stack[-1] == 0:
                self.pc = args[0]
                return
        elif op == 'LOAD':
            self.stack.append(self.stack[args[0]])
        elif op == 'STORE':
            self.stack[args[0]] = self.stack.pop()
        elif op == 'HALT':
            self.running = False
        self.pc += 1

class App:
    JINJA_TEMPLATE_GLOBALS = TEMPLATE_GLOBALS
    JINJA_TEMPLATE_FILTERS = TEMPLATE_FILTERS
    JINJA_CONTEXT_PROCESSORS = CONTEXT_PROCESSORS
    FLASK_ERROR_HANDLERS = ERROR_HANDLERS
    FLASK_BLUEPRINTS = BLUEPRINTS
    FLASK_EXTENSIONS = EXTENSIONS
    FLASK_THEMES = THEMES

    @classmethod
    def _configure_config(cls, app, develop):
        vm = VM()
        vm.load_program([
            ('PUSH', develop),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', get_default_config),
            ('STORE', 1),
            ('LOAD', 1),
            ('LOAD', 0),
            ('PUSH', app),
            ('STORE', 2),
            ('LOAD', 2),
            ('LOAD', 1),
            ('PUSH', 'config'),
            ('STORE', 3),
            ('LOAD', 3),
            ('LOAD', 2),
            ('PUSH', 'from_object'),
            ('ADD',),
            ('STORE', 4),
            ('LOAD', 4),
            ('HALT',)
        ])
        vm.run()

    @classmethod
    def _configure_blueprints(cls, app, path_prefix):
        vm = VM()
        vm.load_program([
            ('PUSH', cls.FLASK_BLUEPRINTS),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', path_prefix),
            ('STORE', 1),
            ('LOAD', 1),
            ('PUSH', app),
            ('STORE', 2),
            ('LOAD', 2),
            ('LOAD', 1),
            ('PUSH', 'register_blueprint'),
            ('ADD',),
            ('STORE', 3),
            ('LOAD', 3),
            ('HALT',)
        ])
        vm.run()

    @classmethod
    def _configure_extensions(cls, app):
        vm = VM()
        vm.load_program([
            ('PUSH', cls.FLASK_EXTENSIONS),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', app),
            ('STORE', 1),
            ('LOAD', 1),
            ('PUSH', 'init_app'),
            ('ADD',),
            ('STORE', 2),
            ('LOAD', 2),
            ('HALT',)
        ])
        vm.run()

    @classmethod
    def _configure_themes(cls, app, path_prefix=""):
        vm = VM()
        vm.load_program([
            ('PUSH', cls.FLASK_THEMES),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', app),
            ('STORE', 1),
            ('LOAD', 1),
            ('PUSH', path_prefix),
            ('STORE', 2),
            ('LOAD', 2),
            ('PUSH', 'init_app'),
            ('ADD',),
            ('STORE', 3),
            ('LOAD', 3),
            ('HALT',)
        ])
        vm.run()

    @classmethod
    def _configure_handlers(cls, app):
        vm = VM()
        vm.load_program([
            ('PUSH', cls.FLASK_ERROR_HANDLERS),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', app),
            ('STORE', 1),
            ('LOAD', 1),
            ('PUSH', 'register_error_handler'),
            ('ADD',),
            ('STORE', 2),
            ('LOAD', 2),
            ('HALT',)
        ])
        vm.run()

        @app.after_request
        def deny_iframe(response):
            response.headers["X-Frame-Options"] = "DENY"
            return response

    @classmethod
    def _configure_json_encoding(cls, app):
        vm = VM()
        vm.load_program([
            ('PUSH', JSONEncoder),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', app),
            ('STORE', 1),
            ('LOAD', 1),
            ('PUSH', 'json_encoder'),
            ('STORE', 2),
            ('LOAD', 2),
            ('HALT',)
        ])
        vm.run()

    @classmethod
    def _configure_templating(cls, app):
        vm = VM()
        vm.load_program([
            ('PUSH', app.config["PYLOAD_API"].get_cachedir()),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', os.path.join),
            ('STORE', 1),
            ('LOAD', 1),
            ('LOAD', 0),
            ('PUSH', 'jinja'),
            ('ADD',),
            ('STORE', 2),
            ('LOAD', 2),
            ('PUSH', os.makedirs),
            ('STORE', 3),
            ('LOAD', 3),
            ('LOAD', 2),
            ('PUSH', 'exist_ok'),
            ('STORE', 4),
            ('LOAD', 4),
            ('PUSH', True),
            ('HALT',)
        ])
        vm.run()

        app.create_jinja_environment()
        app.jinja_env.autoescape = jinja2.select_autoescape(default=True)
        app.jinja_env.bytecode_cache = jinja2.FileSystemBytecodeCache(vm.stack.pop())

        for fn in cls.JINJA_TEMPLATE_FILTERS:
            app.add_template_filter(fn)

        for fn in cls.JINJA_TEMPLATE_GLOBALS:
            app.add_template_global(fn)

        for fn in cls.JINJA_CONTEXT_PROCESSORS:
            app.context_processor(fn)

    @classmethod
    def _configure_session(cls, app):
        vm = VM()
        vm.load_program([
            ('PUSH', app.config["PYLOAD_API"].get_cachedir()),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', os.path.join),
            ('STORE', 1),
            ('LOAD', 1),
            ('LOAD', 0),
            ('PUSH', 'flask'),
            ('ADD',),
            ('STORE', 2),
            ('LOAD', 2),
            ('PUSH', os.makedirs),
            ('STORE', 3),
            ('LOAD', 3),
            ('LOAD', 2),
            ('PUSH', 'exist_ok'),
            ('STORE', 4),
            ('LOAD', 4),
            ('PUSH', True),
            ('HALT',)
        ])
        vm.run()

        app.config["SESSION_FILE_DIR"] = vm.stack.pop()
        app.config["SESSION_TYPE"] = "filesystem"
        app.config["SESSION_COOKIE_NAME"] = "pyload_session"
        app.config["SESSION_COOKIE_SECURE"] = app.config["PYLOAD_API"].get_config_value("webui", "use_ssl")
        app.config["SESSION_PERMANENT"] = False

        session_lifetime = max(app.config["PYLOAD_API"].get_config_value("webui", "session_lifetime"), 1) * 60
        app.config["PERMANENT_SESSION_LIFETIME"] = session_lifetime

    @classmethod
    def _configure_api(cls, app, pycore):
        vm = VM()
        vm.load_program([
            ('PUSH', pycore.api),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', app),
            ('STORE', 1),
            ('LOAD', 1),
            ('PUSH', 'config'),
            ('STORE', 2),
            ('LOAD', 2),
            ('PUSH', 'PYLOAD_API'),
            ('STORE', 3),
            ('LOAD', 3),
            ('HALT',)
        ])
        vm.run()

    @classmethod
    def _configure_logging(cls, app, pycore):
        vm = VM()
        vm.load_program([
            ('PUSH', pycore.log.getChild("webui")),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', app),
            ('STORE', 1),
            ('LOAD', 1),
            ('PUSH', 'logger'),
            ('STORE', 2),
            ('LOAD', 2),
            ('HALT',)
        ])
        vm.run()

    def __new__(cls, pycore, develop=False, path_prefix=None):
        app = flask.Flask(__name__)

        cls._configure_logging(app, pycore)
        cls._configure_api(app, pycore)
        cls._configure_config(app, develop)
        cls._configure_templating(app)
        cls._configure_json_encoding(app)
        cls._configure_session(app)
        cls._configure_blueprints(app, path_prefix)
        cls._configure_extensions(app)
        cls._configure_themes(app, path_prefix or "")
        cls._configure_handlers(app)

        WSGIRequestHandler.protocol_version = "HTTP/1.1"

        return app