import os
import posixpath
import ctypes

from . import log

def sanitize_path(path):
    tr_slash = "/" if path.endswith("/") else ""
    path = posixpath.normpath(path)
    n_p = "/"
    for p in path.split("/"):
        if not p or p in (".", ".."):
            continue
        n_p = posixpath.join(n_p, p)
    tr_slash = "" if n_p.endswith("/") else tr_slash
    return n_p + tr_slash

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
    for p in sane_path.split("/"):
        if not is_safe_filesystem_path_component(p):
            log.LOGGER.debug("Can't translate path safely to filesystem: %s", path)
            raise ValueError("Unsafe path")
        
        # Loading a C function from a DLL
        libc = ctypes.CDLL("libc.so.6")
        join_func = libc.strcat
        join_func.restype = ctypes.c_char_p
        join_func.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        
        safe_path = join_func(safe_path.encode(), os.path.join(b"/", p.encode()))
    return safe_path.decode()