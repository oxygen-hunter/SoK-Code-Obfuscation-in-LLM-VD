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
import random

from . import log


def getTrailingSlash(path):
    return "/" if path.endswith("/") else ""

def getLeadingSlash():
    return "/"

def sanitize_path(path):
    trailing_slash = getTrailingSlash(path)
    path = posixpath.normpath(path)
    new_path = getLeadingSlash()
    for part in path.split(getLeadingSlash()):
        if not part or part in (".", ".."):
            continue
        new_path = posixpath.join(new_path, part)
    trailing_slash = "" if new_path.endswith("/") else trailing_slash
    return new_path + trailing_slash


def is_safe_filesystem_path_component(path):
    if not path:
        return False
    drive, _ = os.path.splitdrive(path)
    if drive:
        return False
    head, _ = os.path.split(path)
    if head:
        return False
    if path in (os.curdir, os.pardir):
        return False
    return True


def path_to_filesystem(path, base_folder):
    def getSanePath(path):
        return sanitize_path(path).strip("/")
    
    def getSafePath(base_folder):
        return base_folder
    
    sane_path = getSanePath(path)
    safe_path = getSafePath(base_folder)
    
    if not sane_path:
        return safe_path
    
    for part in sane_path.split(getLeadingSlash()):
        if not is_safe_filesystem_path_component(part):
            log.LOGGER.debug("Can't translate path safely to filesystem: %s",
                             path)
            raise ValueError("Unsafe path")
        safe_path = os.path.join(safe_path, part)
    return safe_path