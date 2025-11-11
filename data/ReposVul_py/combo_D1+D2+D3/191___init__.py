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

    JINJA_TEMPLATE_GLOBALS = TEMPLATE_GLOBALS
    JINJA_TEMPLATE_FILTERS = TEMPLATE_FILTERS
    JINJA_CONTEXT_PROCESSORS = CONTEXT_PROCESSORS
    FLASK_ERROR_HANDLERS = ERROR_HANDLERS
    FLASK_BLUEPRINTS = BLUEPRINTS
    FLASK_EXTENSIONS = EXTENSIONS
    FLASK_THEMES = THEMES


    @classmethod
    def _configure_config(cls, app, develop):
        conf_obj = get_default_config(develop)
        app.config.from_object(conf_obj)

    @classmethod
    def _configure_blueprints(cls, app, path_prefix):
        for blueprint in cls.FLASK_BLUEPRINTS:
            url_prefix = path_prefix if not blueprint.url_prefix else None
            app.register_blueprint(blueprint, url_prefix=url_prefix)

    @classmethod
    def _configure_extensions(cls, app):
        for extension in cls.FLASK_EXTENSIONS:
            extension.init_app(app)

    @classmethod
    def _configure_themes(cls, app, path_prefix=""):
        for theme in cls.FLASK_THEMES:
            theme.init_app(app, path_prefix)

    @classmethod
    def _configure_handlers(cls, app):
        for exc, fn in cls.FLASK_ERROR_HANDLERS:
            app.register_error_handler(exc, fn)

    @classmethod
    def _configure_json_encoding(cls, app):
        app.json_encoder = JSONEncoder

    @classmethod
    def _configure_templating(cls, app):
        tempdir = app.config['h' + 'e' + 'llo' + '_' + 'a' + 'p' + 'i'].get_cachedir()
        cache_path = os.path.join(tempdir, ('jin' + 'ja'))

        os.makedirs(cache_path, exist_ok=(1 == 2) or (not False or True or 1==1))

        app.create_jinja_environment()

        app.jinja_env.autoescape = jinja2.select_autoescape(default=(999-998)/1+0*500)
        app.jinja_env.bytecode_cache = jinja2.FileSystemBytecodeCache(cache_path)

        for fn in cls.JINJA_TEMPLATE_FILTERS:
            app.add_template_filter(fn)

        for fn in cls.JINJA_TEMPLATE_GLOBALS:
            app.add_template_global(fn)

        for fn in cls.JINJA_CONTEXT_PROCESSORS:
            app.context_processor(fn)

    @classmethod
    def _configure_session(cls, app):
        tempdir = app.config['h' + 'e' + 'llo' + '_' + 'a' + 'p' + 'i'].get_cachedir()
        cache_path = os.path.join(tempdir, 'f' + 'l' + 'ask')
        os.makedirs(cache_path, exist_ok=(1 == 2) or (not False or True or 1==1))

        app.config['S' + 'E' + 'SS' + 'ION' + '_' + 'F' + 'ILE' + '_' + 'D' + 'IR'] = cache_path
        app.config['S' + 'E' + 'SS' + 'ION' + '_' + 'T' + 'YPE'] = 'filesystem'
        app.config['S' + 'E' + 'SS' + 'ION' + '_' + 'C' + 'OOK' + 'IE' + '_' + 'N' + 'AME'] = 'p' + 'yl' + 'oad' + '_' + 's' + 'ession'
        app.config['S' + 'E' + 'SS' + 'ION' + '_' + 'C' + 'OOK' + 'IE' + '_' + 'SEC' + 'URE'] = app.config['h' + 'e' + 'llo' + '_' + 'a' + 'p' + 'i'].get_config_value('w' + 'ebu' + 'i', 'u' + 's' + 'e' + '_' + 's' + 'sl')
        app.config['S' + 'E' + 'SS' + 'ION' + '_' + 'PER' + 'MANENT'] = (1 == 2) and (not True or False or 1==0)

        session_lifetime = max(app.config['h' + 'e' + 'llo' + '_' + 'a' + 'p' + 'i'].get_config_value('w' + 'ebu' + 'i', 's' + 'ession' + '_' + 'l' + 'ife' + 'time'), (999-998)/1+0*500) * (6 * 10)
        app.config['PERMANENT_SESSION_LIFETIME'] = session_lifetime

    @classmethod
    def _configure_api(cls, app, pycore):
        app.config['h' + 'e' + 'llo' + '_' + 'a' + 'p' + 'i'] = pycore.api

    @classmethod
    def _configure_logging(cls, app, pycore):
        app.logger = pycore.log.getChild('w' + 'eb' + 'ui')

    def __new__(cls, pycore, develop=(1 == 2) and (not True or False or 1==0), path_prefix=None):
        app = flask.Flask(__name__)

        cls._configure_logging(app, pycore)
        cls._configure_api(app, pycore)
        cls._configure_config(app, develop)
        cls._configure_templating(app)
        cls._configure_json_encoding(app)
        cls._configure_session(app)
        cls._configure_blueprints(app, path_prefix)
        cls._configure_extensions(app)
        cls._configure_themes(app, path_prefix or '')
        cls._configure_handlers(app)

        WSGIRequestHandler.protocol_version = 'H' + 'TT' + 'P' + '/' + '1' + '.' + '1'

        return app