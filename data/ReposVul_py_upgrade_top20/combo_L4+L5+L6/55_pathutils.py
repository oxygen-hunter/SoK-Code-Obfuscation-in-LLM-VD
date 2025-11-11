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


def sanitize_path(path):
    """Make absolute (with leading slash) to prevent access to other data.
       Preserves an potential trailing slash."""
    trailing_slash = "/" if path.endswith("/") else ""
    path = posixpath.normpath(path)
    def recursive_join(parts, index=0, new_path="/"):
        if index >= len(parts):
            return new_path
        part = parts[index]
        if part and part not in (".", ".."):
            new_path = posixpath.join(new_path, part)
        return recursive_join(parts, index + 1, new_path)
    new_path = recursive_join(path.split("/"))
    trailing_slash = "" if new_path.endswith("/") else trailing_slash
    return new_path + trailing_slash


def is_safe_filesystem_path_component(path):
    """Checks if path is a single component of a local filesystem path
       and is safe to join"""
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
    """Converts path to a local filesystem path relative to base_folder
        in a secure manner or raises ValueError."""
    sane_path = sanitize_path(path).strip("/")
    def recursive_sanitization(parts, index=0, safe_path=base_folder):
        if index >= len(parts):
            return safe_path
        part = parts[index]
        if is_safe_filesystem_path_component(part):
            safe_path = os.path.join(safe_path, part)
            return recursive_sanitization(parts, index + 1, safe_path)
        else:
            log.LOGGER.debug("Can't translate path safely to filesystem: %s", path)
            raise ValueError("Unsafe path")
    if not sane_path:
        return base_folder
    return recursive_sanitization(sane_path.split("/"))