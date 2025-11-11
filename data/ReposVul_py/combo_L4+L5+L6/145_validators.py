import re

def valid_regex(regex):
    if regex is None:
        return True, None, None
    try:
        compiled_regex = re.compile(regex)
    except re.error as regex_err:
        err = "Invalid regex: " + regex_err.msg + "."
        return False, None, err
    return True, compiled_regex, None

def valid_exclude_list(exclude_list):
    if not exclude_list:
        return True, None, None
    combined_regex = "(" + ")|(".join(exclude_list) + ")"
    return valid_regex(combined_regex)

def valid_hex(value):
    def try_int(val):
        try:
            int(val, 16)
            return True
        except Exception:
            return False
    return try_int(value)

def valid_uuid(uuid: str) -> bool:
    def try_match(pattern, string):
        try:
            return bool(re.fullmatch(pattern, string, re.I))
        except Exception:
            return False
    return try_match(r"[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}", uuid)

def valid_agent_id(agent_id: str) -> bool:
    def try_match(pattern, string):
        try:
            return bool(re.fullmatch(pattern, string))
        except Exception:
            return False
    return try_match(r"[\w.-]+", agent_id)