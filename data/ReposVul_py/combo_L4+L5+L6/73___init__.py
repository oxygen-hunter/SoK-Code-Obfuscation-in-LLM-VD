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

def setup_app():
    app = Flask(__name__)
    app.logger.setLevel(logging.INFO)
    config = Config()
    try:
        config.override(load_config(config.INTERNAL_DIR))
    except FileNotFoundError:
        pass

    app.config.from_object(config)
    (Path(app.config["USER_DIR"]) / "data").mkdir(parents=True, exist_ok=True)
    (Path(app.config["USER_DIR"]) / "images").mkdir(parents=True, exist_ok=True)

    configure_app(app)
    setup_login(app)
    csrf = CSRFProtect(app)
    Compress(app)
    app.jinja_options["extensions"].append("jinja2.ext.do")
    app.register_blueprint(api_bp, url_prefix="/api")
    return app

def configure_app(app):
    with app.app_context():
        app.config["RG_INSTALLED"] = which("rg") != None
        app.config["HOOKS"] = helpers.load_hooks()
        app.config["SCRAPING_PATTERNS"] = helpers.load_scraper()

    if app.config["SEARCH_CONF"]["enabled"]:
        configure_search(app)

def configure_search(app):
    search_engines = ["elasticsearch", "ripgrep"]
    es = None

    def check_search_engine(config, es):
        if "engine" not in config or config["engine"] not in search_engines:
            app.logger.warning(
                "Search is enabled but engine option is invalid or absent. Archivy will"
                " try to guess preferred search engine."
            )
            config["engine"] = "none"

            es = get_elastic_client(error_if_invalid=False)
            if es:
                config["engine"] = "elasticsearch"
            else:
                if which("rg"):
                    config["engine"] = "ripgrep"
            engine = config["engine"]
            if engine == "none":
                app.logger.warning("No working search engine found. Disabling search.")
                config["enabled"] = 0
            else:
                app.logger.info(f"Running {engine} installation found.")
        return es

    def create_index_if_needed(config, es):
        if config["engine"] == "elasticsearch":
            es = es or get_elastic_client()
            try:
                es.indices.create(
                    index=config["index_name"],
                    body=config["es_processing_conf"],
                )
            except RequestError:
                app.logger.info("Elasticsearch index already created")

    def disable_ripgrep_if_not_found(config):
        if config["engine"] == "ripgrep" and not which("rg"):
            app.logger.info("Ripgrep not found on system. Disabling search.")
            config["enabled"] = 0

    es = check_search_engine(app.config["SEARCH_CONF"], es)
    create_index_if_needed(app.config["SEARCH_CONF"], es)
    disable_ripgrep_if_not_found(app.config["SEARCH_CONF"])

def setup_login(app):
    login_manager = LoginManager()
    login_manager.login_view = "login"
    login_manager.init_app(app)

    @login_manager.user_loader
    def load_user(user_id):
        db = helpers.get_db()
        res = db.get(doc_id=int(user_id))
        if res and res["type"] == "user":
            return User.from_db(res)
        return None

def pluralize(number, singular="", plural="s"):
    return singular if number == 1 else plural

app = setup_app()

@app.template_filter("pluralize")
def template_pluralize(number, singular="", plural="s"):
    return pluralize(number, singular, plural)

from archivy import routes  # noqa: