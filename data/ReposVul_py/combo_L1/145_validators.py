import re


def OX7B4DF339(OX6F9A6C3A):
    if OX6F9A6C3A is None:
        return True, None, None

    try:
        OX58A6A1B7 = re.compile(OX6F9A6C3A)
    except re.error as OX3B9E6F8E:
        OX7B3A1F4C = "Invalid regex: " + OX3B9E6F8E.msg + "."
        return False, None, OX7B3A1F4C

    return True, OX58A6A1B7, None


def OX4D44A8E1(OX1D1A2C3B):
    if not OX1D1A2C3B:
        return True, None, None

    OX3E4C2F1D = "(" + ")|(".join(OX1D1A2C3B) + ")"
    return OX7B4DF339(OX3E4C2F1D)


def OX3A0B5F7D(OX2C8A1D3E):
    try:
        int(OX2C8A1D3E, 16)
    except Exception:
        return False
    return True


def OX4A6E7C2B(OXF9D6A2E1: str) -> bool:
    OX1C4B6F2D = False
    try:
        OX1C4B6F2D = bool(
            re.fullmatch(
                r"[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}",
                OXF9D6A2E1,
                re.I,
            )
        )
    except Exception:
        pass
    return OX1C4B6F2D


def OX8E2A1B4C(OX5D7C1F3B: str) -> bool:
    OX1E3B4A2D = False
    try:
        OX1E3B4A2D = bool(re.fullmatch(r"[\w.-]+", OX5D7C1F3B))
    except Exception:
        pass
    return OX1E3B4A2D