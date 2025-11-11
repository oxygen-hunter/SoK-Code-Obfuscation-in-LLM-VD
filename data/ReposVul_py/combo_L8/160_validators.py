import validators
from ctypes import CDLL, c_char_p, c_bool

# C code compiled into a shared library (e.g., libvalidate.so)
# C function signatures:
# bool validate_email_c(const char* email);
# bool validate_link_c(const char* link);

lib = CDLL('./libvalidate.so')

def validate_email(email):
    if not isinstance(email, str):
        return False
    validate_email_c = lib.validate_email_c
    validate_email_c.argtypes = [c_char_p]
    validate_email_c.restype = c_bool
    return validate_email_c(email.encode('utf-8'))

def validate_link(link):
    if not isinstance(link, str):
        return False
    validate_link_c = lib.validate_link_c
    validate_link_c.argtypes = [c_char_p]
    validate_link_c.restype = c_bool
    return validate_link_c(link.encode('utf-8'))