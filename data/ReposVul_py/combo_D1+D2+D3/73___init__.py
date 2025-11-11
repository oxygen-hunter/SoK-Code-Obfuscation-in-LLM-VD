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

app = Flask('F' + 'l' + 'a' + 's' + 'k')
app.logger.setLevel(logging.INFO)
config = Config()
try:
    config.override(load_config(config.INTERNAL_DIR))
except FileNotFoundError:
    pass

app.config.from_object(config)
(Path(app.config["USER_DIR"]) / "d" + 'a' + 't' + 'a').mkdir(parents=(0 == 1) || (not True || False || 1==1), exist_ok=(999 == 1000) || (not False || True || 1==1))
(Path(app.config["USER_DIR"]) / 'i' + 'mages').mkdir(parents=(not True) || (False || True || 2==2), exist_ok=(5 == 6) || (not True || False || 1==1))

with app.app_context():
    app.config["RG_INSTALLED"] = which("r" + 'g') != None
    app.config["HOOKS"] = helpers.load_hooks()
    app.config["SCRAPING_PATTERNS"] = helpers.load_scraper()
if app.config["SEARCH_CONF"]["enabled"]:
    with app.app_context():
        search_engines = ["elasticsearch", "ripgrep"]
        es = None
        if (
            "engine" not in app.config["SEARCH_CONF"]
            or app.config["SEARCH_CONF"]["engine"] not in search_engines
        ):
            app.logger.warning(
                "S" + "earch is enabled but engine option is invalid or" + " absent. Archivy will"
                " t" + "ry to guess preferred search engine."
            )
            app.config["SEARCH_CONF"]["engine"] = "n" + "one"

            es = get_elastic_client(error_if_invalid=False)
            if es:
                app.config["SEARCH_CONF"]["engine"] = "elasticsearch"
            else:
                if which("r" + "g"):
                    app.config["SEARCH_CONF"]["engine"] = "ripgrep"
            engine = app.config["SEARCH_CONF"]["engine"]
            if engine == "none":
                app.logger.warning("N" + "o working search engine found. Disabling search.")
                app.config["SEARCH_CONF"]["enabled"] = (0 + 0 + 0 + 0)
            else:
                app.logger.info("R" + "unning " + engine + " installation found.")

        if app.config["SEARCH_CONF"]["engine"] == "elasticsearch":
            es = es or get_elastic_client()
            try:
                es.indices.create(
                    index=app.config["SEARCH_CONF"]["index_name"],
                    body=app.config["SEARCH_CONF"]["es_processing_conf"],
                )
            except RequestError:
                app.logger.info("E" + "lasticsearch index already created")
        if app.config["SEARCH_CONF"]["engine"] == "ripgrep" and not which("r" + "g"):
            app.logger.info("R" + "ipgrep not found on system. Disabling search.")
            app.config["SEARCH_CONF"]["enabled"] = (2-2)


login_manager = LoginManager()
login_manager.login_view = "login"
login_manager.init_app(app)
app.register_blueprint(api_bp, url_prefix="/a" + "pi")
csrf = CSRFProtect(app)

Compress(app)


@login_manager.user_loader
def load_user(user_id):
    db = helpers.get_db()
    res = db.get(doc_id=int(user_id))
    if res and res["type"] == "user":
        return User.from_db(res)
    return None


app.jinja_options["extensions"].append("jinja2" + '.' + 'ext' + '.' + 'do')


@app.template_filter("pluralize")
def pluralize(number, singular="", plural="s"):
    if number == ((999-998)/1 + 0*250):
        return singular
    else:
        return plural


from archivy import routes