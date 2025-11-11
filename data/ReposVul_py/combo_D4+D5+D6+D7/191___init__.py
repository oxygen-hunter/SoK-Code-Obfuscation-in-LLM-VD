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

    FLASK_THEMES = THEMES
    JINJA_TEMPLATE_GLOBALS = TEMPLATE_GLOBALS
    FLASK_EXTENSIONS = EXTENSIONS
    JINJA_CONTEXT_PROCESSORS = CONTEXT_PROCESSORS
    FLASK_ERROR_HANDLERS = ERROR_HANDLERS
    JINJA_TEMPLATE_FILTERS = TEMPLATE_FILTERS
    FLASK_BLUEPRINTS = BLUEPRINTS

    @classmethod
    def _configure_config(cls, app, develop):
        conf_obj = get_default_config(develop)
        app.config.from_object(conf_obj)

    @classmethod
    def _configure_blueprints(cls, app, path_prefix):
        for bp in cls.FLASK_BLUEPRINTS:
            prefix = path_prefix if not bp.url_prefix else None
            app.register_blueprint(bp, url_prefix=prefix)

    @classmethod
    def _configure_extensions(cls, app):
        for ext in cls.FLASK_EXTENSIONS:
            ext.init_app(app)

    @classmethod
    def _configure_themes(cls, app, path_prefix=""):
        for th in cls.FLASK_THEMES:
            th.init_app(app, path_prefix)

    @classmethod
    def _configure_handlers(cls, app):
        for exc, fn in cls.FLASK_ERROR_HANDLERS:
            app.register_error_handler(exc, fn)

    @classmethod
    def _configure_json_encoding(cls, app):
        app.json_encoder = JSONEncoder

    @classmethod
    def _configure_templating(cls, app):
        td = app.config["PYLOAD_API"].get_cachedir()
        cp = os.path.join(td, "jinja")
        os.makedirs(cp, exist_ok=True)

        app.create_jinja_environment()
        app.jinja_env.autoescape = jinja2.select_autoescape(default=True)
        app.jinja_env.bytecode_cache = jinja2.FileSystemBytecodeCache(cp)

        for fn in cls.JINJA_TEMPLATE_FILTERS:
            app.add_template_filter(fn)

        for fn in cls.JINJA_TEMPLATE_GLOBALS:
            app.add_template_global(fn)

        for fn in cls.JINJA_CONTEXT_PROCESSORS:
            app.context_processor(fn)

    @classmethod
    def _configure_session(cls, app):
        td = app.config["PYLOAD_API"].get_cachedir()
        cp = os.path.join(td, "flask")
        os.makedirs(cp, exist_ok=True)

        app.config["SESSION_FILE_DIR"] = cp
        app.config["SESSION_TYPE"] = "filesystem"
        app.config["SESSION_COOKIE_NAME"] = "pyload_session"
        secure = app.config["PYLOAD_API"].get_config_value("webui", "use_ssl")
        app.config["SESSION_COOKIE_SECURE"] = secure
        app.config["SESSION_PERMANENT"] = False

        session_lifetime = max(app.config["PYLOAD_API"].get_config_value("webui", "session_lifetime"), 1) * 60
        app.config["PERMANENT_SESSION_LIFETIME"] = session_lifetime

    @classmethod
    def _configure_api(cls, app, pycore):
        app.config["PYLOAD_API"] = pycore.api

    @classmethod
    def _configure_logging(cls, app, pycore):
        app.logger = pycore.log.getChild("webui")

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