import logging
from pathlib import Path
from shutil import which
from elasticsearch.exceptions import RequestError
from flask import Flask
from flask_compress import Compress
from flask_login import LoginManager
from flask_wtf.csrf import CSRFProtect
from archivy import helpers
from archivy.api import api_bp
from archivy.models import User
from archivy.config import Config
from archivy.helpers import load_config, get_elastic_client

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.running = True
        self.instructions = []
        self.env = {}

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.running:
            self.dispatch()

    def dispatch(self):
        instr = self.instructions[self.pc]
        self.pc += 1
        getattr(self, f"op_{instr[0]}")(*instr[1:])

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_LOAD(self, var_name):
        self.stack.append(self.env[var_name])

    def op_STORE(self, var_name):
        self.env[var_name] = self.stack.pop()

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a - b)

    def op_JMP(self, pc):
        self.pc = pc

    def op_JZ(self, pc):
        if self.op_POP() == 0:
            self.pc = pc

    def op_CALL(self, func_name):
        getattr(self, f"func_{func_name}")()

    def op_HALT(self):
        self.running = False

    def func_setup_app(self):
        app = Flask(__name__)
        app.logger.setLevel(logging.INFO)
        self.env['app'] = app
        self.op_PUSH(app)

    def func_load_config(self):
        config = Config()
        try:
            config.override(load_config(config.INTERNAL_DIR))
        except FileNotFoundError:
            pass
        self.env['config'] = config
        self.op_PUSH(config)

    def func_configure_app(self):
        app = self.op_POP()
        config = self.op_POP()
        app.config.from_object(config)
        (Path(app.config["USER_DIR"]) / "data").mkdir(parents=True, exist_ok=True)
        (Path(app.config["USER_DIR"]) / "images").mkdir(parents=True, exist_ok=True)
        self.op_PUSH(app)

    def func_setup_search(self):
        app = self.op_POP()
        app.config["RG_INSTALLED"] = which("rg") != None
        app.config["HOOKS"] = helpers.load_hooks()
        app.config["SCRAPING_PATTERNS"] = helpers.load_scraper()
        if app.config["SEARCH_CONF"]["enabled"]:
            search_engines = ["elasticsearch", "ripgrep"]
            es = None
            if (
                "engine" not in app.config["SEARCH_CONF"]
                or app.config["SEARCH_CONF"]["engine"] not in search_engines
            ):
                app.logger.warning(
                    "Search is enabled but engine option is invalid or absent. Archivy will"
                    " try to guess preferred search engine."
                )
                app.config["SEARCH_CONF"]["engine"] = "none"

                es = get_elastic_client(error_if_invalid=False)
                if es:
                    app.config["SEARCH_CONF"]["engine"] = "elasticsearch"
                else:
                    if which("rg"):
                        app.config["SEARCH_CONF"]["engine"] = "ripgrep"
                engine = app.config["SEARCH_CONF"]["engine"]
                if engine == "none":
                    app.logger.warning("No working search engine found. Disabling search.")
                    app.config["SEARCH_CONF"]["enabled"] = 0
                else:
                    app.logger.info(f"Running {engine} installation found.")

            if app.config["SEARCH_CONF"]["engine"] == "elasticsearch":
                es = es or get_elastic_client()
                try:
                    es.indices.create(
                        index=app.config["SEARCH_CONF"]["index_name"],
                        body=app.config["SEARCH_CONF"]["es_processing_conf"],
                    )
                except RequestError:
                    app.logger.info("Elasticsearch index already created")
            if app.config["SEARCH_CONF"]["engine"] == "ripgrep" and not which("rg"):
                app.logger.info("Ripgrep not found on system. Disabling search.")
                app.config["SEARCH_CONF"]["enabled"] = 0
        self.op_PUSH(app)

    def func_finalize_app(self):
        app = self.op_POP()
        login_manager = LoginManager()
        login_manager.login_view = "login"
        login_manager.init_app(app)
        app.register_blueprint(api_bp, url_prefix="/api")
        csrf = CSRFProtect(app)
        Compress(app)
        app.jinja_options["extensions"].append("jinja2.ext.do")

        @login_manager.user_loader
        def load_user(user_id):
            db = helpers.get_db()
            res = db.get(doc_id=int(user_id))
            if res and res["type"] == "user":
                return User.from_db(res)
            return None

        @app.template_filter("pluralize")
        def pluralize(number, singular="", plural="s"):
            if number == 1:
                return singular
            else:
                return plural

        self.env['app'] = app

vm = VM()
program = [
    ("CALL", "setup_app"),
    ("CALL", "load_config"),
    ("CALL", "configure_app"),
    ("CALL", "setup_search"),
    ("CALL", "finalize_app"),
    ("HALT",),
]

vm.load_program(program)
vm.run()