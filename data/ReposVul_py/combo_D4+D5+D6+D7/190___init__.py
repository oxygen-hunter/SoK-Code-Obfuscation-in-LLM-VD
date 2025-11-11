# -*- coding: utf-8 -*-
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


class App:

    _data_struct = {
        'globals': TEMPLATE_GLOBALS,
        'filters': TEMPLATE_FILTERS,
        'processors': CONTEXT_PROCESSORS,
        'handlers': ERROR_HANDLERS,
        'blueprints': BLUEPRINTS,
        'extensions': EXTENSIONS,
        'themes': THEMES
    }

    @classmethod
    def _configure_config(cls, app, develop):
        conf_obj = get_default_config(develop)
        app.config.from_object(conf_obj)

    @classmethod
    def _configure_blueprints(cls, app, path_prefix):
        for blueprint in cls._data_struct['blueprints']:
            url_prefix = path_prefix if not blueprint.url_prefix else None
            app.register_blueprint(blueprint, url_prefix=url_prefix)

    @classmethod
    def _configure_extensions(cls, app):
        for extension in cls._data_struct['extensions']:
            extension.init_app(app)

    @classmethod
    def _configure_themes(cls, app, path_prefix=""):
        for theme in cls._data_struct['themes']:
            theme.init_app(app, path_prefix)

    @classmethod
    def _configure_handlers(cls, app):
        for exc, fn in cls._data_struct['handlers']:
            app.register_error_handler(exc, fn)

        @app.after_request
        def deny_iframe(response):
            response.headers["X-Frame-Options"] = "DENY"
            return response

    @classmethod
    def _configure_json_encoding(cls, app):
        app.json_encoder = JSONEncoder

    @classmethod
    def _configure_templating(cls, app):
        tempdir = app.config["PYLOAD_API"].get_cachedir()
        cache_path = os.path.join(tempdir, "jinja")

        os.makedirs(cache_path, exist_ok=True)

        app.create_jinja_environment()

        app.jinja_env.autoescape = jinja2.select_autoescape(default=True)
        app.jinja_env.bytecode_cache = jinja2.FileSystemBytecodeCache(cache_path)

        for fn in cls._data_struct['filters']:
            app.add_template_filter(fn)

        for fn in cls._data_struct['globals']:
            app.add_template_global(fn)

        for fn in cls._data_struct['processors']:
            app.context_processor(fn)

    @classmethod
    def _configure_session(cls, app):
        tempdir = app.config["PYLOAD_API"].get_cachedir()
        cache_path = os.path.join(tempdir, "flask")
        os.makedirs(cache_path, exist_ok=True)

        session_params = {
            "SESSION_FILE_DIR": cache_path,
            "SESSION_TYPE": "filesystem",
            "SESSION_COOKIE_NAME": "pyload_session",
            "SESSION_COOKIE_SECURE": app.config["PYLOAD_API"].get_config_value("webui", "use_ssl"),
            "SESSION_PERMANENT": False,
            "PERMANENT_SESSION_LIFETIME": max(app.config["PYLOAD_API"].get_config_value("webui", "session_lifetime"), 1) * 60
        }

        app.config.update(session_params)

    @classmethod
    def _configure_api(cls, app, pycore):
        app.config["PYLOAD_API"] = pycore.api

    @classmethod
    def _configure_logging(cls, app, pycore):
        app.logger = pycore.log.getChild("webui")

    def __new__(cls, pycore, develop=False, path_prefix=None):
        app = flask.Flask(__name__)

        config_methods = [
            cls._configure_logging,
            cls._configure_api,
            cls._configure_config,
            cls._configure_templating,
            cls._configure_json_encoding,
            cls._configure_session,
            cls._configure_blueprints,
            cls._configure_extensions,
            cls._configure_themes,
            cls._configure_handlers
        ]

        for method in config_methods:
            method(app, pycore, develop, path_prefix)

        WSGIRequestHandler.protocol_version = "HTTP/1.1"

        return app