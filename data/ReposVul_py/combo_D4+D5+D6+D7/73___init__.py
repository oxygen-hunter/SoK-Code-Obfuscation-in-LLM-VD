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
conf = Config()
try:
    conf.override(load_config(conf.INTERNAL_DIR))
except FileNotFoundError:
    pass

app.config.from_object(conf)
user_dir = Path(app.config["USER_DIR"])
data_dir = user_dir / "data"
images_dir = user_dir / "images"
data_dir.mkdir(parents=True, exist_ok=True)
images_dir.mkdir(parents=True, exist_ok=True)

with app.app_context():
    rg_installed = which("rg") != None
    app.config["RG_INSTALLED"] = rg_installed
    hooks = helpers.load_hooks()
    scraping_patterns = helpers.load_scraper()
    app.config["HOOKS"], app.config["SCRAPING_PATTERNS"] = hooks, scraping_patterns

if app.config["SEARCH_CONF"]["enabled"]:
    with app.app_context():
        search_engines = ["elasticsearch", "ripgrep"]
        es_client = None
        if (
            "engine" not in app.config["SEARCH_CONF"]
            or app.config["SEARCH_CONF"]["engine"] not in search_engines
        ):
            app.logger.warning(
                "Search is enabled but engine option is invalid or absent. Archivy will"
                " try to guess preferred search engine."
            )
            app.config["SEARCH_CONF"]["engine"] = "none"

            es_client = get_elastic_client(error_if_invalid=False)
            if es_client:
                app.config["SEARCH_CONF"]["engine"] = "elasticsearch"
            else:
                if which("rg"):
                    app.config["SEARCH_CONF"]["engine"] = "ripgrep"
            search_engine = app.config["SEARCH_CONF"]["engine"]
            if search_engine == "none":
                app.logger.warning("No working search engine found. Disabling search.")
                app.config["SEARCH_CONF"]["enabled"] = 0
            else:
                app.logger.info(f"Running {search_engine} installation found.")

        if app.config["SEARCH_CONF"]["engine"] == "elasticsearch":
            es_client = es_client or get_elastic_client()
            try:
                es_client.indices.create(
                    index=app.config["SEARCH_CONF"]["index_name"],
                    body=app.config["SEARCH_CONF"]["es_processing_conf"],
                )
            except RequestError:
                app.logger.info("Elasticsearch index already created")
        if app.config["SEARCH_CONF"]["engine"] == "ripgrep" and not which("rg"):
            app.logger.info("Ripgrep not found on system. Disabling search.")
            app.config["SEARCH_CONF"]["enabled"] = 0


login_manager = LoginManager()
login_manager.login_view = "login"
login_manager.init_app(app)
app.register_blueprint(api_bp, url_prefix="/api")
csrf = CSRFProtect(app)

Compress(app)

@login_manager.user_loader
def load_user(user_id):
    db_instance = helpers.get_db()
    result = db_instance.get(doc_id=int(user_id))
    if result and result["type"] == "user":
        return User.from_db(result)
    return None

app.jinja_options["extensions"].append("jinja2.ext.do")

@app.template_filter("pluralize")
def pluralize(number, singular="", plural="s"):
    singular_plural_pair = [singular, plural]
    return singular_plural_pair[number != 1]

from archivy import routes  # noqa: