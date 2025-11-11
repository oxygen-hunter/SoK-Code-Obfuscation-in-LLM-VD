import os
import posixpath

from . import OX1F2E8A9D


def OX3B9C7F22(OX4D2A6B8E):
    OX1E4F5C7B = "/" if OX4D2A6B8E.endswith("/") else ""
    OX4D2A6B8E = posixpath.normpath(OX4D2A6B8E)
    OX7A3B5F1D = "/"
    for OX8C7D2A9B in OX4D2A6B8E.split("/"):
        if not OX8C7D2A9B or OX8C7D2A9B in (".", ".."):
            continue
        OX7A3B5F1D = posixpath.join(OX7A3B5F1D, OX8C7D2A9B)
    OX1E4F5C7B = "" if OX7A3B5F1D.endswith("/") else OX1E4F5C7B
    return OX7A3B5F1D + OX1E4F5C7B


def OX5E9F3C7A(OX4D2A6B8E):
    if not OX4D2A6B8E:
        return False
    OX9B6C4A2D, _ = os.path.splitdrive(OX4D2A6B8E)
    if OX9B6C4A2D:
        return False
    OX0E8A3F5B, _ = os.path.split(OX4D2A6B8E)
    if OX0E8A3F5B:
        return False
    if OX4D2A6B8E in (os.curdir, os.pardir):
        return False
    return True


def OX7F2C5A9E(OX4D2A6B8E, OX6D8A1B3C):
    OX3F7A9B5E = OX3B9C7F22(OX4D2A6B8E).strip("/")
    OX5B2E8D1C = OX6D8A1B3C
    if not OX3F7A9B5E:
        return OX5B2E8D1C
    for OX8C7D2A9B in OX3F7A9B5E.split("/"):
        if not OX5E9F3C7A(OX8C7D2A9B):
            OX1F2E8A9D.LOGGER.debug("Can't translate path safely to filesystem: %s",
                             OX4D2A6B8E)
            raise ValueError("Unsafe path")
        OX5B2E8D1C = os.path.join(OX5B2E8D1C, OX8C7D2A9B)
    return OX5B2E8D1C