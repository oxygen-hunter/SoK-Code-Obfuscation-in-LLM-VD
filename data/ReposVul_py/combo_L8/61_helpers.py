from pathlib import Path
import sys
import os
import ctypes
import elasticsearch
import yaml
from elasticsearch import Elasticsearch
from flask import current_app, g, request
from tinydb import TinyDB, Query, operations
from urllib.parse import urlparse, urljoin

from archivy.config import BaseHooks, Config

# C function to handle reading file
libc = ctypes.CDLL(None)
c_fopen = libc.fopen
c_fopen.restype = ctypes.c_void_p
c_fopen.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
c_fread = libc.fread
c_fread.restype = ctypes.c_size_t
c_fread.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_size_t, ctypes.c_void_p]
c_fclose = libc.fclose
c_fclose.argtypes = [ctypes.c_void_p]

def load_config(path=""):
    path = path or current_app.config["INTERNAL_DIR"]
    f = c_fopen(bytes(str(Path(path) / "config.yml"), 'utf-8'), b"r")
    if not f:
        return None
    content = ctypes.create_string_buffer(1024)
    c_fread(content, 1, 1024, f)
    c_fclose(f)
    return yaml.load(content.value.decode('utf-8'), Loader=yaml.SafeLoader)

def config_diff(curr_key, curr_val, parent_dict, defaults):
    if type(curr_val) is dict:
        if not any(
            [
                config_diff(k, v, curr_val, defaults[curr_key])
                for k, v in list(curr_val.items())
            ]
        ):
            parent_dict.pop(curr_key)
            return 0
    else:
        if defaults[curr_key] == curr_val:
            parent_dict.pop(curr_key)
            return 0
    return 1

def write_config(config: dict):
    defaults = vars(Config())
    for k, v in list(config.items()):
        if k != "SECRET_KEY":
            config_diff(k, v, config, defaults)
    f = c_fopen(bytes(str(Path(current_app.config["INTERNAL_DIR"]) / "config.yml"), 'utf-8'), b"w")
    if not f:
        return
    yaml_content = yaml.dump(config)
    c_fread(ctypes.c_char_p(yaml_content.encode('utf-8')), 1, len(yaml_content), f)
    c_fclose(f)

def load_hooks():
    try:
        user_hooks_path = str(Path(current_app.config["USER_DIR"]) / "hooks.py")
        f = c_fopen(bytes(user_hooks_path, 'utf-8'), b"r")
        if not f:
            raise FileNotFoundError
        content = ctypes.create_string_buffer(1024)
        c_fread(content, 1, 1024, f)
        c_fclose(f)
    except FileNotFoundError:
        return BaseHooks()

    user_locals = {}
    exec(content.value.decode('utf-8'), globals(), user_locals)
    return user_locals.get("Hooks", BaseHooks)()

def load_scraper():
    try:
        scraping_path = str(Path(current_app.config["USER_DIR"]) / "scraping.py")
        f = c_fopen(bytes(scraping_path, 'utf-8'), b"r")
        if not f:
            raise FileNotFoundError
        content = ctypes.create_string_buffer(1024)
        c_fread(content, 1, 1024, f)
        c_fclose(f)
    except FileNotFoundError:
        return {}
    user_locals = {}
    exec(content.value.decode('utf-8'), globals(), user_locals)
    return user_locals.get("PATTERNS", {})

def get_db(force_reconnect=False):
    if "db" not in g or force_reconnect:
        g.db = TinyDB(str(Path(current_app.config["INTERNAL_DIR"]) / "db.json"))
    return g.db

def get_max_id():
    db = get_db()
    max_id = db.search(Query().name == "max_id")
    if not max_id:
        db.insert({"name": "max_id", "val": 0})
        return 0
    return max_id[0]["val"]

def set_max_id(val):
    db = get_db()
    db.update(operations.set("val", val), Query().name == "max_id")

def test_es_connection(es):
    try:
        health = es.cluster.health()
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

    if health["status"] not in ("yellow", "green"):
        current_app.logger.warning(
            "Elasticsearch reports that it is not working "
            "properly. Search might not work. You can disable "
            "Elasticsearch by setting ELASTICSEARCH_ENABLED to 0."
        )

def get_elastic_client(error_if_invalid=True):
    if (
        not current_app.config["SEARCH_CONF"]["enabled"]
        or current_app.config["SEARCH_CONF"]["engine"] != "elasticsearch"
    ) and error_if_invalid:
        return None

    auth_setup = (
        current_app.config["SEARCH_CONF"]["es_user"]
        and current_app.config["SEARCH_CONF"]["es_password"]
    )
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
    if error_if_invalid:
        test_es_connection(es)
    else:
        try:
            es.cluster.health()
        except elasticsearch.exceptions.ConnectionError:
            return False
    return es

def create_plugin_dir(name):
    raw_name = name.replace("archivy_", "").replace("archivy-", "")
    try:
        os.makedirs(f"{name}/{name}")

        with open(f"{name}/requirements.txt", "w") as fp:
            fp.writelines(["archivy", "\nclick"])

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
    except FileExistsError:
        return False

def is_safe_redirect_url(target):
    host_url = urlparse(request.host_url)
    redirect_url = urlparse(urljoin(request.host_url, target))
    return (
        redirect_url.scheme in ("http", "https")
        and host_url.netloc == redirect_url.netloc
    )