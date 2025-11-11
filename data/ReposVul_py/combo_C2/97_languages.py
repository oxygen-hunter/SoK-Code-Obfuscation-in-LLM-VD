__author__ = "Gina Häußge <osd@foosel.net>"
__license__ = "GNU Affero General Public License http://www.gnu.org/licenses/agpl.html"
__copyright__ = "Copyright (C) 2015 The OctoPrint Project - Released under terms of the AGPLv3 License"

import logging
import os
import tarfile
import zipfile
from collections import defaultdict

from flask import abort, jsonify, request
from flask_babel import Locale

from octoprint.access.permissions import Permissions
from octoprint.plugin import plugin_manager
from octoprint.server.api import api
from octoprint.server.util.flask import no_firstrun_access
from octoprint.settings import settings
from octoprint.util import yaml


@api.route("/languages", methods=["GET"])
@no_firstrun_access
@Permissions.SETTINGS.require(403)
def getInstalledLanguagePacks():
    translation_folder = settings().getBaseFolder("translations", check_writable=False)
    
    state = "start"
    while True:
        if state == "start":
            if not os.path.exists(translation_folder):
                return jsonify(language_packs={"_core": []})

            core_packs = []
            plugin_packs = defaultdict(
                lambda: {"identifier": None, "display": None, "languages": []}
            )
            entries = os.scandir(translation_folder)
            state = "process_entries"
        
        elif state == "process_entries":
            try:
                entry = next(entries)
                state = "check_entry_dir"
            except StopIteration:
                state = "prepare_result"
        
        elif state == "check_entry_dir":
            if not entry.is_dir():
                state = "process_entries"
            else:
                state = "check_entry_name"
        
        elif state == "check_entry_name":
            if entry.name == "_plugins":
                plugin_entries = os.scandir(entry.path)
                state = "process_plugin_entries"
            else:
                try:
                    core_packs.append(load_meta(entry.path, entry.name))
                except ValueError:
                    logging.getLogger(__name__).exception(
                        "Core language pack {} doesn't appear to actually be one".format(
                            entry.name
                        )
                    )
                except Exception:
                    logging.getLogger(__name__).exception(
                        "Error while parsing metadata for core language pack {} from {}".format(
                            entry.name, entry.path
                        )
                    )
                state = "process_entries"
        
        elif state == "process_plugin_entries":
            try:
                plugin_entry = next(plugin_entries)
                state = "check_plugin_entry_dir"
            except StopIteration:
                state = "process_entries"
        
        elif state == "check_plugin_entry_dir":
            if not plugin_entry.is_dir():
                state = "process_plugin_entries"
            else:
                state = "check_plugin_in_manager"
        
        elif state == "check_plugin_in_manager":
            if plugin_entry.name not in plugin_manager().plugins:
                state = "process_plugin_entries"
            else:
                plugin_info = plugin_manager().plugins[plugin_entry.name]
                plugin_packs[plugin_entry.name]["identifier"] = plugin_entry.name
                plugin_packs[plugin_entry.name]["display"] = plugin_info.name
                language_entries = os.scandir(plugin_entry.path)
                state = "process_language_entries"
        
        elif state == "process_language_entries":
            try:
                language_entry = next(language_entries)
                state = "append_language"
            except StopIteration:
                state = "process_plugin_entries"
        
        elif state == "append_language":
            try:
                plugin_packs[plugin_entry.name]["languages"].append(
                    load_meta(language_entry.path, language_entry.name)
                )
            except Exception:
                logging.getLogger(__name__).exception(
                    "Error while parsing metadata for language pack {} from {} for plugin {}".format(
                        language_entry.name,
                        language_entry.path,
                        plugin_entry.name,
                    )
                )
            state = "process_language_entries"
        
        elif state == "prepare_result":
            result = {
                "_core": {"identifier": "_core", "display": "Core", "languages": core_packs}
            }
            result.update(plugin_packs)
            return jsonify(language_packs=result)


def load_meta(path, locale):
    state = "start"
    while True:
        if state == "start":
            meta = {}
            meta_path = os.path.join(path, "meta.yaml")
            state = "check_file_exists"

        elif state == "check_file_exists":
            if os.path.isfile(meta_path):
                try:
                    meta = yaml.load_from_file(path=meta_path)
                except Exception:
                    logging.getLogger(__name__).exception("Could not load %s", meta_path)
                else:
                    import datetime

                    if "last_update" in meta and isinstance(
                        meta["last_update"], datetime.datetime
                    ):
                        meta["last_update"] = (
                            meta["last_update"] - datetime.datetime(1970, 1, 1)
                        ).total_seconds()
            loc = Locale.parse(locale)
            meta["locale"] = locale
            meta["locale_display"] = loc.display_name
            meta["locale_english"] = loc.english_name
            return meta


@api.route("/languages", methods=["POST"])
@no_firstrun_access
@Permissions.SETTINGS.require(403)
def uploadLanguagePack():
    state = "start"
    while True:
        if state == "start":
            input_name = "file"
            input_upload_path = (
                input_name + "." + settings().get(["server", "uploads", "pathSuffix"])
            )
            input_upload_name = (
                input_name + "." + settings().get(["server", "uploads", "nameSuffix"])
            )
            state = "check_request_values"

        elif state == "check_request_values":
            if input_upload_path not in request.values or input_upload_name not in request.values:
                abort(400, description="No file included")
            upload_name = request.values[input_upload_name]
            upload_path = request.values[input_upload_path]
            state = "filter_extensions"

        elif state == "filter_extensions":
            exts = list(
                filter(
                    lambda x: upload_name.lower().endswith(x), (".zip", ".tar.gz", ".tgz", ".tar")
                )
            )
            if not len(exts):
                abort(
                    400,
                    description="File doesn't have a valid extension for a language pack archive",
                )
            target_path = settings().getBaseFolder("translations")
            state = "check_file_type"

        elif state == "check_file_type":
            if tarfile.is_tarfile(upload_path):
                _unpack_uploaded_tarball(upload_path, target_path)
            elif zipfile.is_zipfile(upload_path):
                _unpack_uploaded_zipfile(upload_path, target_path)
            else:
                abort(400, description="Neither zip file nor tarball included")
            return getInstalledLanguagePacks()


@api.route("/languages/<string:locale>/<string:pack>", methods=["DELETE"])
@no_firstrun_access
@Permissions.SETTINGS.require(403)
def deleteInstalledLanguagePack(locale, pack):
    state = "start"
    while True:
        if state == "start":
            if pack == "_core":
                target_path = os.path.join(settings().getBaseFolder("translations"), locale)
            else:
                target_path = os.path.join(
                    settings().getBaseFolder("translations"), "_plugins", pack, locale
                )
            state = "check_dir"

        elif state == "check_dir":
            if os.path.isdir(target_path):
                import shutil
                shutil.rmtree(target_path)
            return getInstalledLanguagePacks()


def _unpack_uploaded_zipfile(path, target):
    with zipfile.ZipFile(path, "r") as zip:
        state = "start"
        while True:
            if state == "start":
                infos = zip.infolist()
                state = "validate_info"

            elif state == "validate_info":
                for info in infos:
                    _validate_zip_info(info, target)
                state = "extract"

            elif state == "extract":
                zip.extractall(target)
                break


def _unpack_uploaded_tarball(path, target):
    with tarfile.open(path, "r") as tar:
        state = "start"
        while True:
            if state == "start":
                infos = tar.getmembers()
                state = "validate_info"

            elif state == "validate_info":
                for info in infos:
                    _validate_tar_info(info, target)
                state = "extract"

            elif state == "extract":
                tar.extractall(target)
                break


def _validate_archive_name(name, target):
    if not os.path.abspath(os.path.join(target, name)).startswith(target + os.path.sep):
        raise InvalidLanguagePack(f"Provided language pack contains invalid name {name}")


def _validate_zip_info(info, target):
    _validate_archive_name(info.filename, target)


def _validate_tar_info(info, target):
    _validate_archive_name(info.name, target)
    if not (info.isfile() or info.isdir()):
        raise InvalidLanguagePack("Provided language pack contains invalid file type")


class InvalidLanguagePack(Exception):
    pass