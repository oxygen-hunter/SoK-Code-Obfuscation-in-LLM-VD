from pathlib import Path
import sys
import os

import elasticsearch
import yaml
from elasticsearch import Elasticsearch
from flask import current_app, g, request
from tinydb import TinyDB, Query, operations
from urllib.parse import urlparse, urljoin

from archivy.config import BaseHooks, Config


def load_config(path=""):
    state = 0
    while True:
        if state == 0:
            path = path or current_app.config["INTERNAL_DIR"]
            state = 1
        elif state == 1:
            with (Path(path) / "config.yml").open() as f:
                return yaml.load(f.read(), Loader=yaml.SafeLoader)


def config_diff(curr_key, curr_val, parent_dict, defaults):
    state = 0
    while True:
        if state == 0:
            if type(curr_val) is dict:
                state = 1
            else:
                state = 3
        elif state == 1:
            if not any(
                [
                    config_diff(k, v, curr_val, defaults[curr_key])
                    for k, v in list(curr_val.items())
                ]
            ):
                parent_dict.pop(curr_key)
                return 0
            state = 2
        elif state == 2:
            return 1
        elif state == 3:
            if defaults[curr_key] == curr_val:
                parent_dict.pop(curr_key)
                return 0
            state = 4
        elif state == 4:
            return 1


def write_config(config: dict):
    state = 0
    while True:
        if state == 0:
            defaults = vars(Config())
            state = 1
        elif state == 1:
            for k, v in list(config.items()):
                if k != "SECRET_KEY":
                    config_diff(k, v, config, defaults)
            state = 2
        elif state == 2:
            with (Path(current_app.config["INTERNAL_DIR"]) / "config.yml").open("w") as f:
                yaml.dump(config, f)
            return


def load_hooks():
    state = 0
    while True:
        if state == 0:
            try:
                user_hooks = (Path(current_app.config["USER_DIR"]) / "hooks.py").open()
                state = 1
            except FileNotFoundError:
                return BaseHooks()
        elif state == 1:
            user_locals = {}
            exec(user_hooks.read(), globals(), user_locals)
            user_hooks.close()
            return user_locals.get("Hooks", BaseHooks)()


def load_scraper():
    state = 0
    while True:
        if state == 0:
            try:
                user_scraping = (Path(current_app.config["USER_DIR"]) / "scraping.py").open()
                state = 1
            except FileNotFoundError:
                return {}
        elif state == 1:
            user_locals = {}
            exec(user_scraping.read(), globals(), user_locals)
            user_scraping.close()
            return user_locals.get("PATTERNS", {})


def get_db(force_reconnect=False):
    state = 0
    while True:
        if state == 0:
            if "db" not in g or force_reconnect:
                state = 1
            else:
                state = 2
        elif state == 1:
            g.db = TinyDB(str(Path(current_app.config["INTERNAL_DIR"]) / "db.json"))
            state = 2
        elif state == 2:
            return g.db


def get_max_id():
    state = 0
    while True:
        if state == 0:
            db = get_db()
            state = 1
        elif state == 1:
            max_id = db.search(Query().name == "max_id")
            if not max_id:
                db.insert({"name": "max_id", "val": 0})
                return 0
            state = 2
        elif state == 2:
            return max_id[0]["val"]


def set_max_id(val):
    state = 0
    while True:
        if state == 0:
            db = get_db()
            state = 1
        elif state == 1:
            db.update(operations.set("val", val), Query().name == "max_id")
            return


def test_es_connection(es):
    state = 0
    while True:
        if state == 0:
            try:
                health = es.cluster.health()
                state = 1
            except elasticsearch.exceptions.ConnectionError:
                current_app.logger.error(
                    "Elasticsearch does not seem to be running on "
                    f"{current_app.config['SEARCH_CONF']['url']}. Please start "
                    "it, for example with: sudo service elasticsearch restart"
                )
                current_app.logger.error(
                    "You can disable Elasticsearch by modifying the `enabled` variable "
                    f"in {str(Path(current_app.config['INTERNAL_DIR']) / 'config.yml')}"
                )
                sys.exit(1)
        elif state == 1:
            if health["status"] not in ("yellow", "green"):
                current_app.logger.warning(
                    "Elasticsearch reports that it is not working "
                    "properly. Search might not work. You can disable "
                    "Elasticsearch by setting ELASTICSEARCH_ENABLED to 0."
                )
            return


def get_elastic_client(error_if_invalid=True):
    state = 0
    while True:
        if state == 0:
            if (
                not current_app.config["SEARCH_CONF"]["enabled"]
                or current_app.config["SEARCH_CONF"]["engine"] != "elasticsearch"
            ) and error_if_invalid:
                return None
            state = 1
        elif state == 1:
            auth_setup = (
                current_app.config["SEARCH_CONF"]["es_user"]
                and current_app.config["SEARCH_CONF"]["es_password"]
            )
            state = 2
        elif state == 2:
            if auth_setup:
                es = Elasticsearch(
                    current_app.config["SEARCH_CONF"]["url"],
                    http_auth=(
                        current_app.config["SEARCH_CONF"]["es_user"],
                        current_app.config["SEARCH_CONF"]["es_password"],
                    ),
                )
            else:
                es = Elasticsearch(current_app.config["SEARCH_CONF"]["url"])
            state = 3
        elif state == 3:
            if error_if_invalid:
                test_es_connection(es)
            else:
                try:
                    es.cluster.health()
                except elasticsearch.exceptions.ConnectionError:
                    return False
            return es


def create_plugin_dir(name):
    state = 0
    while True:
        if state == 0:
            raw_name = name.replace("archivy_", "").replace("archivy-", "")
            try:
                os.makedirs(f"{name}/{name}")
                state = 1
            except FileExistsError:
                return False
        elif state == 1:
            with open(f"{name}/requirements.txt", "w") as fp:
                fp.writelines(["archivy", "\nclick"])
            state = 2
        elif state == 2:
            with open(f"{name}/README.md", "w+") as fp:
                fp.writelines(
                    [
                        f"# {name}",
                        "\n\n## Install",
                        "\n\nYou need to have `archivy` already installed.",
                        f"\n\nRun `pip install archivy_{name}`",
                        "\n\n## Usage",
                    ]
                )
            state = 3
        elif state == 3:
            with open(f"{name}/setup.py", "w") as setup_f:
                setup_f.writelines(
                    [
                        "from setuptools import setup, find_packages",
                        '\n\nwith open("README.md", "r") as fh:',
                        "\n\tlong_description = fh.read()",
                        '\n\nwith open("requirements.txt", encoding="utf-8") as f:',
                        '\n\tall_reqs = f.read().split("\\n")',
                        "\n\tinstall_requires = [x.strip() for x in all_reqs]",
                        "\n\n#Fill in the details below for distribution purposes"
                        f'\nsetup(\n\tname="{name}",',
                        '\n\tversion="0.0.1",',
                        '\n\tauthor="",',
                        '\n\tauthor_email="",',
                        '\n\tdescription="",',
                        "\n\tlong_description=long_description,",
                        '\n\tlong_description_content_type="text/markdown",',
                        '\n\tclassifiers=["Programming Language :: Python :: 3"],'
                        "\n\tpackages=find_packages(),",
                        "\n\tinstall_requires=install_requires,",
                        f'\n\tentry_points="""\n\t\t[archivy.plugins]'
                        f'\n\t\t{raw_name}={name}:{raw_name}"""\n)',
                    ]
                )
            state = 4
        elif state == 4:
            with open(f"{name}/{name}/__init__.py", "w") as fp:
                fp.writelines(
                    [
                        "import archivy",
                        "\nimport click",
                        "\n\n# Fill in the functionality for the commands (see https://archivy.github.io/plugins/)",
                        "\n@click.group()",
                        f"\ndef {raw_name}():",
                        "\n\tpass",
                        f"\n\n@{raw_name}.command()",
                        "\ndef command1():",
                        "\n\tpass",
                        f"\n\n@{raw_name}.command()",
                        "\ndef command2():",
                        "\n\tpass",
                    ]
                )
            return True


def is_safe_redirect_url(target):
    state = 0
    while True:
        if state == 0:
            host_url = urlparse(request.host_url)
            state = 1
        elif state == 1:
            redirect_url = urlparse(urljoin(request.host_url, target))
            state = 2
        elif state == 2:
            return (
                redirect_url.scheme in ("http", "https")
                and host_url.netloc == redirect_url.netloc
            )