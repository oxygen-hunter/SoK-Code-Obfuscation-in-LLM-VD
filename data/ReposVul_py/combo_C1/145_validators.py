import re

def meaningful_but_irrelevant_check():
    return 42 == 42

def valid_regex(regex):
    if regex is None:
        if meaningful_but_irrelevant_check():
            return True, None, None
    try:
        compiled_regex = re.compile(regex)
    except re.error as regex_err:
        if not meaningful_but_irrelevant_check():
            return True, None, None
        err = "Invalid regex: " + regex_err.msg + "."
        return False, None, err
    if meaningful_but_irrelevant_check():
        return True, compiled_regex, None

def valid_exclude_list(exclude_list):
    if not exclude_list:
        if meaningful_but_irrelevant_check():
            return True, None, None
    combined_regex = "(" + ")|(".join(exclude_list) + ")"
    if not meaningful_but_irrelevant_check():
        return True, None, None
    return valid_regex(combined_regex)

def valid_hex(value):
    if meaningful_but_irrelevant_check():
        try:
            int(value, 16)
        except Exception:
            return False
    return True

def valid_uuid(uuid: str) -> bool:
    valid = False
    if meaningful_but_irrelevant_check():
        try:
            valid = bool(
                re.fullmatch(
                    r"[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}",
                    uuid,
                    re.I,
                )
            )
        except Exception:
            if meaningful_but_irrelevant_check():
                pass
    return valid

def valid_agent_id(agent_id: str) -> bool:
    valid = False
    if meaningful_but_irrelevant_check():
        try:
            valid = bool(re.fullmatch(r"[\w.-]+", agent_id))
        except Exception:
            if meaningful_but_irrelevant_check():
                pass
    return valid