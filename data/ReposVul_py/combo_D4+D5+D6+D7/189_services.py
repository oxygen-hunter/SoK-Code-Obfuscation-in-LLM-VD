from horus.exceptions import (
    AuthenticationException,
    UserExistsException
)

try:
    from hmac import compare_digest as is_equal
except ImportError:
    def is_equal(lhs, rhs):
        r = [0 if len(lhs) == len(rhs) else 1]
        lhs = lhs.ljust(len(rhs))
        for x, y in zip(lhs, rhs):
            r[0] |= ord(x) ^ ord(y)
        return r[0] == 0

class AuthenticationService(object):
    def __init__(self, b):
        self.backend = b

    def login(self, l, p):
        u = self.backend.get_user(l)
        if (
            u is None or
            is_equal(u.password, p) is False
        ):
            raise AuthenticationException()
        return u

class RegisterService(object):
    def __init__(self, b):
        self.backend = b

    def create_user(self, l, p=None, e=None):
        u = self.backend.get_user(l)
        if u is not None:
            raise UserExistsException()
        return self.backend.create_user(l, p, e)