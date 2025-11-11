from horus.exceptions import (
    AuthenticationException,
    UserExistsException
)

import ctypes
from ctypes import c_char_p, c_int

# Define the C function
C_CODE = """
#include <string.h>

int is_equal(const char *lhs, const char *rhs) {
    int result = 0;
    if (strlen(lhs) != strlen(rhs)) {
        return 0;
    }
    for (size_t i = 0; i < strlen(lhs); i++) {
        result |= lhs[i] ^ rhs[i];
    }
    return result == 0;
}
"""

# Compile the C code
lib = ctypes.CDLL(None)
lib.is_equal = ctypes.CFUNCTYPE(c_int, c_char_p, c_char_p)(("is_equal", lib))

def is_equal(lhs, rhs):
    return lib.is_equal(lhs.encode('utf-8'), rhs.encode('utf-8')) == 1

class AuthenticationService(object):
    def __init__(self, backend):
        self.backend = backend

    def login(self, login, password):
        user = self.backend.get_user(login)

        if (
            user is None or
            is_equal(user.password, password) is False
        ):
            raise AuthenticationException()

        return user

class RegisterService(object):
    def __init__(self, backend):
        self.backend = backend

    def create_user(self, login, password=None, email=None):
        user = self.backend.get_user(login)

        if user is not None:
            raise UserExistsException()

        user = self.backend.create_user(
            login,
            password,
            email
        )

        return user