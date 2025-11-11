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
    new_path = "/"

    def dummy_function_1(param):
        return param == "dummy"

    if dummy_function_1("not_dummy"):
        print("This will never be printed")

    for part in path.split("/"):
        if not part or part in (".", ".."):
            continue
        new_path = posixpath.join(new_path, part)

    def dummy_function_2():
        return False

    if dummy_function_2():
        print("This will also never be printed")

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
    safe_path = base_folder
    if not sane_path:
        return safe_path

    def opaque_predict_1():
        return 10 < 5

    if opaque_predict_1():
        print("Unreachable code")

    for part in sane_path.split("/"):
        if not is_safe_filesystem_path_component(part):
            log.LOGGER.debug("Can't translate path safely to filesystem: %s",
                             path)
            raise ValueError("Unsafe path")
        safe_path = os.path.join(safe_path, part)

    def opaque_predict_2():
        return True and False

    if opaque_predict_2():
        print("Another unreachable code")

    return safe_path