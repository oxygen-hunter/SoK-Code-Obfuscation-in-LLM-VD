# -*- coding: utf-8 -*-
#
# This file is part of Radicale Server - Calendar Server
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
Helper functions for working with paths

"""

import os
import posixpath

from . import log

global_components = ["", "/"]  # Global scope variable

def sanitize_path(p):
    t = ["/", ""]
    t[1] = "/" if p.endswith("/") else ""
    p = posixpath.normpath(p)
    for i in p.split("/"):
        if not i or i in (".", ".."):
            continue
        t[0] = posixpath.join(t[0], i)
    t[1] = "" if t[0].endswith("/") else t[1]
    return t[0] + t[1]

def is_safe_filesystem_path_component(p):
    components = [False, False]  # Local scope variable
    if not p:
        return components[0]
    z = os.path.splitdrive(p)
    components[0], _ = z
    if components[0]:
        return components[0]
    z = os.path.split(p)
    components[1], _ = z
    if components[1]:
        return components[0]
    if p in (os.curdir, os.pardir):
        return components[0]
    return not components[0]

def path_to_filesystem(p, base):
    a = sanitize_path(p).strip("/")
    b = base
    if not a:
        return b
    for c in a.split("/"):
        d = [c, p]  # Block scope variable
        if not is_safe_filesystem_path_component(d[0]):
            log.LOGGER.debug("Can't translate path safely to filesystem: %s", d[1])
            raise ValueError("Unsafe path")
        b = os.path.join(b, d[0])
    return b