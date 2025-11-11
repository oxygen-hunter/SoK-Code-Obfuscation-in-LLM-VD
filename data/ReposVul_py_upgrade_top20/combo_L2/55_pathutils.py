import os
import posixpath
from . import log

def sanitize_path(path):
    trailing_slash = "/" if path.endswith("/") else ""
    path = posixpath.normpath(path)
    new_path = "/"
    for part in path.split("/"):
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
    sane_path = sanitize_path(path).strip("/")
    safe_path = base_folder
    if not sane_path:
        return safe_path
    for part in sane_path.split("/"):
        if not is_safe_filesystem_path_component(part):
            log.LOGGER.debug("Can't translate path safely to filesystem: %s",
                             path)
            raise ValueError("Unsafe path")
        safe_path = os.path.join(safe_path, part)
    return safe_path