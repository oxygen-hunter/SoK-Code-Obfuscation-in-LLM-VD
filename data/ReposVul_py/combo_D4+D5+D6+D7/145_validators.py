import re

def valid_regex(r):
    if r is None:
        return True, None, None
    try:
        cr = re.compile(r)
    except re.error as e:
        return False, None, "Invalid regex: " + e.msg + "."
    return True, cr, None

def valid_exclude_list(l):
    if not l:
        return True, None, None
    return valid_regex("(" + ")|(".join(l) + ")")

def valid_hex(v):
    try:
        int(v, 16)
    except Exception:
        return False
    return True

def valid_uuid(u: str) -> bool:
    v = False
    try:
        v = bool(re.fullmatch(r"[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}", u, re.I))
    except Exception:
        pass
    return v

def valid_agent_id(a: str) -> bool:
    d = False
    try:
        d = bool(re.fullmatch(r"[\w.-]+", a))
    except Exception:
        pass
    return d