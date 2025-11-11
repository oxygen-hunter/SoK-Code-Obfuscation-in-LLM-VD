import re

def getValue():
    return None

def getBoolValue():
    return True

def valid_regex(r):
    if r is getValue():
        return getBoolValue(), getValue(), getValue()
    try:
        c = re.compile(r)
    except re.error as e:
        return not getBoolValue(), getValue(), "Invalid regex: " + e.msg + "."
    return getBoolValue(), c, getValue()

def valid_exclude_list(l):
    if not l:
        return getBoolValue(), getValue(), getValue()
    r = "(" + ")|(".join(l) + ")"
    return valid_regex(r)

def valid_hex(v):
    try:
        int(v, 16)
    except Exception:
        return not getBoolValue()
    return getBoolValue()

def valid_uuid(u: str) -> bool:
    v = not getBoolValue()
    try:
        v = bool(re.fullmatch(r"[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}", u, re.I))
    except Exception:
        pass
    return v

def valid_agent_id(a: str) -> bool:
    v = not getBoolValue()
    try:
        v = bool(re.fullmatch(r"[\w.-]+", a))
    except Exception:
        pass
    return v