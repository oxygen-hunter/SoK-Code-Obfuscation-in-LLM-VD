import re
import ctypes

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
    try:
        int(value, 16)
    except Exception:
        return False
    return True

def valid_uuid(uuid: str) -> bool:
    valid = False
    try:
        valid = bool(
            re.fullmatch(
                r"[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}",
                uuid,
                re.I,
            )
        )
    except Exception:
        pass
    return valid

def valid_agent_id(agent_id: str) -> bool:
    valid = False
    try:
        valid = bool(re.fullmatch(r"[\w.-]+", agent_id))
    except Exception:
        pass
    return valid

def check_hex(value):
    return ctypes.CDLL(None).atoi(value.encode(), 16) != 0

def main():
    # Inline assembly with ctypes
    lib = ctypes.CDLL(None)
    lib.printf(b"Valid Hex: %d\n", check_hex(b"1f"))
    lib.printf(b"Valid UUID: %d\n", valid_uuid("f47ac10b-58cc-4372-a567-0e02b2c3d479"))
    lib.printf(b"Valid Agent ID: %d\n", valid_agent_id("agent-007"))

if __name__ == "__main__":
    main()