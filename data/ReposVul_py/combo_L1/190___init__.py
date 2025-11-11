import os

import flask
import jinja2
from werkzeug.serving import WSGIRequestHandler

from .blueprints import BLUEPRINTS as OX5C6F0E49
from .config import get_default_config as OX4A8E9F62
from .extensions import EXTENSIONS as OX7B3D1C8A, THEMES as OX3A4E7C9D
from .filters import TEMPLATE_FILTERS as OX1F9E0D7B
from .globals import TEMPLATE_GLOBALS as OX9E0B6A5C
from .handlers import ERROR_HANDLERS as OX4B2D8A7E
from .helpers import JSONEncoder as OX6D4F3B2A
from .processors import CONTEXT_PROCESSORS as OX8A3C5E1F

class OX7A9B2C3D:
    OX2B1C3D4E = OX9E0B6A5C
    OX5F4E3D2C = OX1F9E0D7B
    OX8C7B6A5D = OX8A3C5E1F
    OX3F2E1D0C = OX4B2D8A7E
    OX6A5B4C3D = OX5C6F0E49
    OX9D8E7F6C = OX7B3D1C8A
    OX1C2D3E4B = OX3A4E7C9D

    @classmethod
    def OX0F1E2D3C(cls, OX2E1D0C3B, OX5C4B3A2D):
        OX9F8E7D6C = OX4A8E9F62(OX5C4B3A2D)
        OX2E1D0C3B.config.from_object(OX9F8E7D6C)

    @classmethod
    def OX3B2A1D0C(cls, OX6C5B4A3D, OX2F1E0D3C):
        for OX4E3D2C1B in cls.OX6A5B4C3D:
            OX1D0C3B2A = OX2F1E0D3C if not OX4E3D2C1B.url_prefix else None
            OX6C5B4A3D.register_blueprint(OX4E3D2C1B, url_prefix=OX1D0C3B2A)

    @classmethod
    def OX5D4C3B2A(cls, OX8A7B6C5D):
        for OX0F1E2D3C in cls.OX9D8E7F6C:
            OX0F1E2D3C.init_app(OX8A7B6C5D)

    @classmethod
    def OX7E6F5D4C(cls, OX2B1C0D3A, OX7A8B9C0D=""):
        for OX4C3D2A1B in cls.OX1C2D3E4B:
            OX4C3D2A1B.init_app(OX2B1C0D3A, OX7A8B9C0D)

    @classmethod
    def OX9F8E7D6C(cls, OX5A4B3C2D):
        for OX2B1C0D3A, OX1E0F9D8C in cls.OX3F2E1D0C:
            OX5A4B3C2D.register_error_handler(OX2B1C0D3A, OX1E0F9D8C)

        @OX5A4B3C2D.after_request
        def OX3D2C1B0A(OX7E6F5D4C):
            OX7E6F5D4C.headers["X-Frame-Options"] = "DENY"
            return OX7E6F5D4C

    @classmethod
    def OX2F1E0D3C(cls, OX8A7B6C5D):
        OX8A7B6C5D.json_encoder = OX6D4F3B2A

    @classmethod
    def OX4B3A2C1D(cls, OX0F9E8D7C):
        OX6C5B4A3D = OX0F9E8D7C.config["PYLOAD_API"].get_cachedir()
        OX5A4B3C2D = os.path.join(OX6C5B4A3D, "jinja")

        os.makedirs(OX5A4B3C2D, exist_ok=True)

        OX0F9E8D7C.create_jinja_environment()

        OX0F9E8D7C.jinja_env.autoescape = jinja2.select_autoescape(default=True)
        OX0F9E8D7C.jinja_env.bytecode_cache = jinja2.FileSystemBytecodeCache(OX5A4B3C2D)

        for OX9F8E7D6C in cls.OX5F4E3D2C:
            OX0F9E8D7C.add_template_filter(OX9F8E7D6C)

        for OX9F8E7D6C in cls.OX2B1C3D4E:
            OX0F9E8D7C.add_template_global(OX9F8E7D6C)

        for OX9F8E7D6C in cls.OX8C7B6A5D:
            OX0F9E8D7C.context_processor(OX9F8E7D6C)

    @classmethod
    def OX6F5D4C3B(cls, OX1B0A9C8D):
        OX5D4C3B2A = OX1B0A9C8D.config["PYLOAD_API"].get_cachedir()
        OX4E3D2C1B = os.path.join(OX5D4C3B2A, "flask")
        os.makedirs(OX4E3D2C1B, exist_ok=True)

        OX1B0A9C8D.config["SESSION_FILE_DIR"] = OX4E3D2C1B
        OX1B0A9C8D.config["SESSION_TYPE"] = "filesystem"
        OX1B0A9C8D.config["SESSION_COOKIE_NAME"] = "pyload_session"
        OX1B0A9C8D.config["SESSION_COOKIE_SECURE"] = OX1B0A9C8D.config["PYLOAD_API"].get_config_value("webui", "use_ssl")
        OX1B0A9C8D.config["SESSION_PERMANENT"] = False

        OX0F9E8D7C = max(OX1B0A9C8D.config["PYLOAD_API"].get_config_value("webui", "session_lifetime"), 1) * 60
        OX1B0A9C8D.config["PERMANENT_SESSION_LIFETIME"] = OX0F9E8D7C

    @classmethod
    def OX8D7C6B5A(cls, OX2E1D0C3B, OX9F8E7D6C):
        OX2E1D0C3B.config["PYLOAD_API"] = OX9F8E7D6C.api

    @classmethod
    def OX9C8B7A6D(cls, OX8A7B6C5D, OX5D4C3B2A):
        OX8A7B6C5D.logger = OX5D4C3B2A.log.getChild("webui")

    def __new__(cls, OX4B3A2C1D, OX2B1C0D3A=False, OX7A8B9C0D=None):
        OX2E1D0C3B = flask.Flask(__name__)

        cls.OX9C8B7A6D(OX2E1D0C3B, OX4B3A2C1D)
        cls.OX8D7C6B5A(OX2E1D0C3B, OX4B3A2C1D)
        cls.OX0F1E2D3C(OX2E1D0C3B, OX2B1C0D3A)
        cls.OX4B3A2C1D(OX2E1D0C3B)
        cls.OX2F1E0D3C(OX2E1D0C3B)
        cls.OX6F5D4C3B(OX2E1D0C3B)
        cls.OX3B2A1D0C(OX2E1D0C3B, OX7A8B9C0D)
        cls.OX5D4C3B2A(OX2E1D0C3B)
        cls.OX7E6F5D4C(OX2E1D0C3B, OX7A8B9C0D or "")
        cls.OX9F8E7D6C(OX2E1D0C3B)

        WSGIRequestHandler.protocol_version = "HTTP/1.1"

        return OX2E1D0C3B