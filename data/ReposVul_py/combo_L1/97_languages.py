__author__ = "Gina Häußge <osd@foosel.net>"
__license__ = "GNU Affero General Public License http://www.gnu.org/licenses/agpl.html"
__copyright__ = "Copyright (C) 2015 The OctoPrint Project - Released under terms of the AGPLv3 License"

import logging as OX2A3E9B8
import os as OX3F4E1C8
import tarfile as OXF9D5A42
import zipfile as OXB2E7A12
from collections import defaultdict as OX8F3D6A9

from flask import abort as OX9E7B2A6, jsonify as OX5C8D7F3, request as OX1A7F9C2
from flask_babel import Locale as OX3F1D6B2

from octoprint.access.permissions import Permissions as OX8A2F1B7
from octoprint.plugin import plugin_manager as OXF4A2E9B
from octoprint.server.api import api as OX7C6F2A3
from octoprint.server.util.flask import no_firstrun_access as OX0D6A3F8
from octoprint.settings import settings as OX9B4E2F1
from octoprint.util import yaml as OX1F3D9A7


@OX7C6F2A3.route("/languages", methods=["GET"])
@OX0D6A3F8
@OX8A2F1B7.SETTINGS.require(403)
def OX7B4DF339():
    OX5A9D2B6 = OX9B4E2F1().getBaseFolder("translations", check_writable=False)
    if not OX3F4E1C8.path.exists(OX5A9D2B6):
        return OX5C8D7F3(language_packs={"_core": []})

    OX2B7C8A9 = []
    OX6E5F1A3 = OX8F3D6A9(
        lambda: {"identifier": None, "display": None, "languages": []}
    )
    for OX9C4E2A7 in OX3F4E1C8.scandir(OX5A9D2B6):
        if not OX9C4E2A7.is_dir():
            continue

        def OX6A9B8F1(OX5E3F2A7, OX4D1C9B2):
            OX4F3A8D2 = {}

            OX9B7E2F4 = OX3F4E1C8.path.join(OX5E3F2A7, "meta.yaml")
            if OX3F4E1C8.path.isfile(OX9B7E2F4):
                try:
                    OX4F3A8D2 = OX1F3D9A7.load_from_file(path=OX9B7E2F4)
                except Exception:
                    OX2A3E9B8.getLogger(__name__).exception("Could not load %s", OX9B7E2F4)
                    pass
                else:
                    import datetime as OX5D7A4B9

                    if "last_update" in OX4F3A8D2 and isinstance(
                        OX4F3A8D2["last_update"], OX5D7A4B9.datetime
                    ):
                        OX4F3A8D2["last_update"] = (
                            OX4F3A8D2["last_update"] - OX5D7A4B9.datetime(1970, 1, 1)
                        ).total_seconds()

            OX2B5F1A8 = OX3F1D6B2.parse(OX4D1C9B2)
            OX4F3A8D2["locale"] = OX4D1C9B2
            OX4F3A8D2["locale_display"] = OX2B5F1A8.display_name
            OX4F3A8D2["locale_english"] = OX2B5F1A8.english_name
            return OX4F3A8D2

        if OX9C4E2A7.name == "_plugins":
            for OX7A2D5B9 in OX3F4E1C8.scandir(OX9C4E2A7.path):
                if not OX7A2D5B9.is_dir():
                    continue

                if OX7A2D5B9.name not in OXF4A2E9B().plugins:
                    continue

                OXD3A5B8C = OXF4A2E9B().plugins[OX7A2D5B9.name]

                OX6E5F1A3[OX7A2D5B9.name]["identifier"] = OX7A2D5B9.name
                OX6E5F1A3[OX7A2D5B9.name]["display"] = OXD3A5B8C.name

                for OX9F1A3B8 in OX3F4E1C8.scandir(OX7A2D5B9.path):
                    try:
                        OX6E5F1A3[OX7A2D5B9.name]["languages"].append(
                            OX6A9B8F1(OX9F1A3B8.path, OX9F1A3B8.name)
                        )
                    except Exception:
                        OX2A3E9B8.getLogger(__name__).exception(
                            "Error while parsing metadata for language pack {} from {} for plugin {}".format(
                                OX9F1A3B8.name,
                                OX9F1A3B8.path,
                                OX7A2D5B9.name,
                            )
                        )
                        continue
        else:
            try:
                OX2B7C8A9.append(OX6A9B8F1(OX9C4E2A7.path, OX9C4E2A7.name))
            except ValueError:
                OX2A3E9B8.getLogger(__name__).exception(
                    "Core language pack {} doesn't appear to actually be one".format(
                        OX9C4E2A7.name
                    )
                )
            except Exception:
                OX2A3E9B8.getLogger(__name__).exception(
                    "Error while parsing metadata for core language pack {} from {}".format(
                        OX9C4E2A7.name, OX9C4E2A7.path
                    )
                )

    OX0B7E9C3 = {
        "_core": {"identifier": "_core", "display": "Core", "languages": OX2B7C8A9}
    }
    OX0B7E9C3.update(OX6E5F1A3)
    return OX5C8D7F3(language_packs=OX0B7E9C3)


@OX7C6F2A3.route("/languages", methods=["POST"])
@OX0D6A3F8
@OX8A2F1B7.SETTINGS.require(403)
def OX8F6A3B9():
    OX5D2B3F6 = "file"
    OX4E7C1A9 = (
        OX5D2B3F6 + "." + OX9B4E2F1().get(["server", "uploads", "pathSuffix"])
    )
    OX1A4E7F2 = (
        OX5D2B3F6 + "." + OX9B4E2F1().get(["server", "uploads", "nameSuffix"])
    )
    if OX4E7C1A9 not in OX1A7F9C2.values or OX1A4E7F2 not in OX1A7F9C2.values:
        OX9E7B2A6(400, description="No file included")

    OX3A5B8C2 = OX1A7F9C2.values[OX1A4E7F2]
    OXC8B2A7E = OX1A7F9C2.values[OX4E7C1A9]

    OX2E1F7A3 = list(
        filter(
            lambda x: OX3A5B8C2.lower().endswith(x), (".zip", ".tar.gz", ".tgz", ".tar")
        )
    )
    if not len(OX2E1F7A3):
        OX9E7B2A6(
            400,
            description="File doesn't have a valid extension for a language pack archive",
        )

    OX9F2D1A3 = OX9B4E2F1().getBaseFolder("translations")

    if OXF9D5A42.is_tarfile(OXC8B2A7E):
        OX5F3B8A1(OXC8B2A7E, OX9F2D1A3)
    elif OXB2E7A12.is_zipfile(OXC8B2A7E):
        OX3D7A5B9(OXC8B2A7E, OX9F2D1A3)
    else:
        OX9E7B2A6(400, description="Neither zip file nor tarball included")

    return OX7B4DF339()


@OX7C6F2A3.route("/languages/<string:locale>/<string:pack>", methods=["DELETE"])
@OX0D6A3F8
@OX8A2F1B7.SETTINGS.require(403)
def OXA9F1D3C(OX5B2F3A8, OX3E7A1D9):

    if OX3E7A1D9 == "_core":
        OX4A7B6E1 = OX3F4E1C8.path.join(OX9B4E2F1().getBaseFolder("translations"), OX5B2F3A8)
    else:
        OX4A7B6E1 = OX3F4E1C8.path.join(
            OX9B4E2F1().getBaseFolder("translations"), "_plugins", OX3E7A1D9, OX5B2F3A8
        )

    if OX3F4E1C8.path.isdir(OX4A7B6E1):
        import shutil as OX6B1D5A3

        OX6B1D5A3.rmtree(OX4A7B6E1)

    return OX7B4DF339()


def OX3D7A5B9(OX2C4E1B8, OX8A5B3F7):
    with OXB2E7A12.ZipFile(OX2C4E1B8, "r") as OX5D3A9B8:
        for OX9B2F3A7 in OX5D3A9B8.infolist():
            OXA5B7C2D(OX9B2F3A7, OX8A5B3F7)

        OX5D3A9B8.extractall(OX8A5B3F7)


def OX5F3B8A1(OX2F4A1B9, OX7E5B3A8):
    with OXF9D5A42.open(OX2F4A1B9, "r") as OXA2B9F1C:
        for OX8C5D3B7 in OXA2B9F1C.getmembers():
            OXA9C5E1F(OX8C5D3B7, OX7E5B3A8)

        OXA2B9F1C.extractall(OX7E5B3A8)


def OX1B8A5C2F(OX3E9D4F2, OX6F2B5A9):
    if not OX3F4E1C8.path.abspath(OX3F4E1C8.path.join(OX6F2B5A9, OX3E9D4F2)).startswith(OX6F2B5A9 + OX3F4E1C8.path.sep):
        raise OX2D9C6A5(f"Provided language pack contains invalid name {OX3E9D4F2}")


def OXA5B7C2D(OX1F4A7B3, OX7D3E2A9):
    OX1B8A5C2F(OX1F4A7B3.filename, OX7D3E2A9)


def OXA9C5E1F(OX2A4B7F9, OX6E2A5D3):
    OX1B8A5C2F(OX2A4B7F9.name, OX6E2A5D3)
    if not (OX2A4B7F9.isfile() or OX2A4B7F9.isdir()):
        raise OX2D9C6A5("Provided language pack contains invalid file type")


class OX2D9C6A5(Exception):
    pass