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

from .. import config, log

if sys.version_info[0] == 2:
    from ConfigParser import ConfigParser
    from StringIO import StringIO
else:
    from configparser import ConfigParser
    from io import StringIO

DEFINED_RIGHTS = {
    "authenticated": "[rw]\nuser:.+\ncollection:.*\npermission:rw",
    "owner_write": "[r]\nuser:.+\ncollection:.*\npermission:r\n"
                   "[w]\nuser:.+\ncollection:^%(login)s(/.*)?$\npermission:w",
    "owner_only": "[rw]\nuser:.+\ncollection:^%(login)s(/.*)?$\npermission:rw",
}

def _read_from_sections(u, c, p):
    f = os.path.expanduser(config.get("rights", "file"))
    r = config.get("rights", "type").lower()
    u_e = re.escape(u)
    c_e = re.escape(c)
    rgx = ConfigParser({"login": u_e, "path": c_e})
    if r in DEFINED_RIGHTS:
        log.LOGGER.debug("Rights type '%s'" % r)
        rgx.readfp(StringIO(DEFINED_RIGHTS[r]))
    elif r == "from_file":
        log.LOGGER.debug("Reading rights from file %s" % f)
        if not rgx.read(f):
            log.LOGGER.error("File '%s' not found for rights" % f)
            return False
    else:
        log.LOGGER.error("Unknown rights type '%s'" % r)
        return False

    for s in rgx.sections():
        r_u = rgx.get(s, "user")
        r_c = rgx.get(s, "collection")
        log.LOGGER.debug(
            "Test if '%s:%s' matches against '%s:%s' from section '%s'" % (
                u, c, r_u, r_c, s))
        u_m = re.match(r_u, u)
        if u_m:
            r_c = r_c.format(*u_m.groups())
            if re.match(r_c, c):
                log.LOGGER.debug("Section '%s' matches" % s)
                if p in rgx.get(s, "permission"):
                    return True
            else:
                log.LOGGER.debug("Section '%s' does not match" % s)
    return False

def authorized(u, clctn, prmssn):
    clctn_l = clctn.url.rstrip("/") or "/"
    if clctn_l in (".well-known/carddav", ".well-known/caldav"):
        return prmssn == "r"
    rt = config.get("rights", "type").lower()
    return (
        rt == "none" or
        _read_from_sections(u or "", clctn_l, prmssn))