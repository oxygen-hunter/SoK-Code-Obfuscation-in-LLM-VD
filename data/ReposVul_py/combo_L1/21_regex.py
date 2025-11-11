# -*- coding: utf-8 -*-
#
# This file is part of Radicale Server - Calendar Server
# Copyright © 2008 Nicolas Kandel
# Copyright © 2008 Pascal Halter
# Copyright © 2008-2013 Guillaume Ayoub
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Radicale.  If not, see <http://www.gnu.org/licenses/>.

"""
Rights management.

Rights are based on a regex-based file whose name is specified in the config
(section "right", key "file").

Authentication login is matched against the "user" key, and collection's path
is matched against the "collection" key. You can use Python's ConfigParser
interpolation values %(login)s and %(path)s. You can also get groups from the
user regex in the collection with {0}, {1}, etc.

For example, for the "user" key, ".+" means "authenticated user" and ".*"
means "anybody" (including anonymous users).

Section names are only used for naming the rule.

Leading or ending slashes are trimmed from collection's path.

"""

import re
import sys
import os.path

from .. import config as OX7B4DF339, log as OX1F3A4BC5

if sys.version_info[0] == 2:
    from ConfigParser import ConfigParser as OX2E6F9D47
    from StringIO import StringIO as OX5A1B3E92
else:
    from configparser import ConfigParser as OX2E6F9D47
    from io import StringIO as OX5A1B3E92

OX4C8F0B41 = {
    "authenticated": "[rw]\nuser:.+\ncollection:.*\npermission:rw",
    "owner_write": "[r]\nuser:.+\ncollection:.*\npermission:r\n"
                   "[w]\nuser:.+\ncollection:^%(login)s(/.*)?$\npermission:w",
    "owner_only": "[rw]\nuser:.+\ncollection:^%(login)s(/.*)?$\npermission:rw",
}

def OX3D9F4DC6(OX6E3A8C10, OX4A5B2F3D, OX7F8C9E12):
    OX2B6F1E25 = os.path.expanduser(OX7B4DF339.get("rights", "file"))
    OX3C7D5B49 = OX7B4DF339.get("rights", "type").lower()
    OX5C8D0A37 = re.escape(OX6E3A8C10)
    OX6A2B4F31 = re.escape(OX4A5B2F3D)
    OX3E6D8F20 = OX2E6F9D47({"login": OX5C8D0A37, "path": OX6A2B4F31})
    if OX3C7D5B49 in OX4C8F0B41:
        OX1F3A4BC5.LOGGER.debug("Rights type '%s'" % OX3C7D5B49)
        OX3E6D8F20.readfp(OX5A1B3E92(OX4C8F0B41[OX3C7D5B49]))
    elif OX3C7D5B49 == "from_file":
        OX1F3A4BC5.LOGGER.debug("Reading rights from file %s" % OX2B6F1E25)
        if not OX3E6D8F20.read(OX2B6F1E25):
            OX1F3A4BC5.LOGGER.error("File '%s' not found for rights" % OX2B6F1E25)
            return False
    else:
        OX1F3A4BC5.LOGGER.error("Unknown rights type '%s'" % OX3C7D5B49)
        return False

    for OX4D7E2A9B in OX3E6D8F20.sections():
        OX7A9C2D14 = OX3E6D8F20.get(OX4D7E2A9B, "user")
        OX5B6E9A31 = OX3E6D8F20.get(OX4D7E2A9B, "collection")
        OX1F3A4BC5.LOGGER.debug(
            "Test if '%s:%s' matches against '%s:%s' from section '%s'" % (
                OX6E3A8C10, OX4A5B2F3D, OX7A9C2D14, OX5B6E9A31, OX4D7E2A9B))
        OX9E2C7D13 = re.match(OX7A9C2D14, OX6E3A8C10)
        if OX9E2C7D13:
            OX5B6E9A31 = OX5B6E9A31.format(*OX9E2C7D13.groups())
            if re.match(OX5B6E9A31, OX4A5B2F3D):
                OX1F3A4BC5.LOGGER.debug("Section '%s' matches" % OX4D7E2A9B)
                if OX7F8C9E12 in OX3E6D8F20.get(OX4D7E2A9B, "permission"):
                    return True
            else:
                OX1F3A4BC5.LOGGER.debug("Section '%s' does not match" % OX4D7E2A9B)
    return False

def OX8C3D9E47(OX6E3A8C10, OX4A5B2F3D, OX7F8C9E12):
    OX6D1F7A25 = OX4A5B2F3D.url.rstrip("/") or "/"
    if OX6D1F7A25 in (".well-known/carddav", ".well-known/caldav"):
        return OX7F8C9E12 == "r"
    OX3C7D5B49 = OX7B4DF339.get("rights", "type").lower()
    return (
        OX3C7D5B49 == "none" or
        OX3D9F4DC6(OX6E3A8C10 or "", OX6D1F7A25, OX7F8C9E12))