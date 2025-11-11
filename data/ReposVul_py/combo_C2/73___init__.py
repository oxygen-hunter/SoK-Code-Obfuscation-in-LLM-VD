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

app = Flask(__name__)
app.logger.setLevel(logging.INFO)
config = Config()
dispatcher = 0

while True:
    if dispatcher == 0:
        try:
            config.override(load_config(config.INTERNAL_DIR))
        except FileNotFoundError:
            pass
        dispatcher = 1
    elif dispatcher == 1:
        app.config.from_object(config)
        (Path(app.config["USER_DIR"]) / "data").mkdir(parents=True, exist_ok=True)
        (Path(app.config["USER_DIR"]) / "images").mkdir(parents=True, exist_ok=True)
        dispatcher = 2
    elif dispatcher == 2:
        with app.app_context():
            app.config["RG_INSTALLED"] = which("rg") != None
            app.config["HOOKS"] = helpers.load_hooks()
            app.config["SCRAPING_PATTERNS"] = helpers.load_scraper()
        dispatcher = 3
    elif dispatcher == 3:
        if app.config["SEARCH_CONF"]["enabled"]:
            dispatcher = 4
        else:
            dispatcher = 8
    elif dispatcher == 4:
        with app.app_context():
            search_engines = ["elasticsearch", "ripgrep"]
            es = None
            if (
                "engine" not in app.config["SEARCH_CONF"]
                or app.config["SEARCH_CONF"]["engine"] not in search_engines
            ):
                dispatcher = 5
            else:
                dispatcher = 6
    elif dispatcher == 5:
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
        dispatcher = 6
    elif dispatcher == 6:
        if app.config["SEARCH_CONF"]["engine"] == "elasticsearch":
            es = es or get_elastic_client()
            try:
                es.indices.create(
                    index=app.config["SEARCH_CONF"]["index_name"],
                    body=app.config["SEARCH_CONF"]["es_processing_conf"],
                )
            except RequestError:
                app.logger.info("Elasticsearch index already created")
        dispatcher = 7
    elif dispatcher == 7:
        if app.config["SEARCH_CONF"]["engine"] == "ripgrep" and not which("rg"):
            app.logger.info("Ripgrep not found on system. Disabling search.")
            app.config["SEARCH_CONF"]["enabled"] = 0
        dispatcher = 8
    elif dispatcher == 8:
        login_manager = LoginManager()
        login_manager.login_view = "login"
        login_manager.init_app(app)
        app.register_blueprint(api_bp, url_prefix="/api")
        csrf = CSRFProtect(app)

        Compress(app)

        @login_manager.user_loader
        def load_user(user_id):
            db = helpers.get_db()
            res = db.get(doc_id=int(user_id))
            if res and res["type"] == "user":
                return User.from_db(res)
            return None

        app.jinja_options["extensions"].append("jinja2.ext.do")

        @app.template_filter("pluralize")
        def pluralize(number, singular="", plural="s"):
            if number == 1:
                return singular
            else:
                return plural

        from archivy import routes  # noqa:
        
        break