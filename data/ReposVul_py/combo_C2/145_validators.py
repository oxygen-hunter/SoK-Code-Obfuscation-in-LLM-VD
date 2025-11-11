import re

def valid_regex(regex):
    state = 0
    while True:
        if state == 0:
            if regex is None:
                return True, None, None
            state = 1
        elif state == 1:
            try:
                compiled_regex = re.compile(regex)
                state = 3
            except re.error as regex_err:
                state = 2
        elif state == 2:
            err = "Invalid regex: " + regex_err.msg + "."
            return False, None, err
        elif state == 3:
            return True, compiled_regex, None

def valid_exclude_list(exclude_list):
    state = 0
    while True:
        if state == 0:
            if not exclude_list:
                return True, None, None
            state = 1
        elif state == 1:
            combined_regex = "(" + ")|(".join(exclude_list) + ")"
            return valid_regex(combined_regex)

def valid_hex(value):
    state = 0
    while True:
        if state == 0:
            try:
                int(value, 16)
                state = 2
            except Exception:
                state = 1
        elif state == 1:
            return False
        elif state == 2:
            return True

def valid_uuid(uuid: str) -> bool:
    state = 0
    valid = False
    while True:
        if state == 0:
            try:
                valid = bool(
                    re.fullmatch(
                        r"[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}",
                        uuid,
                        re.I,
                    )
                )
                state = 2
            except Exception:
                state = 1
        elif state == 1:
            pass
            state = 2
        elif state == 2:
            return valid

def valid_agent_id(agent_id: str) -> bool:
    state = 0
    valid = False
    while True:
        if state == 0:
            try:
                valid = bool(re.fullmatch(r"[\w.-]+", agent_id))
                state = 2
            except Exception:
                state = 1
        elif state == 1:
            pass
            state = 2
        elif state == 2:
            return valid