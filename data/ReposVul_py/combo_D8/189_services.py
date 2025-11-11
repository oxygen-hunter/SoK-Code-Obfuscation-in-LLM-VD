from horus.exceptions import (
    AuthenticationException,
    UserExistsException
)

try:
    from hmac import compare_digest as is_equal
except ImportError:
    def is_equal(lhs, rhs):
        result = 0 if len(lhs) == len(rhs) else 1
        lhs = lhs.ljust(len(rhs))
        for x, y in zip(lhs, rhs):
            result |= ord(x) ^ ord(y)
        return result == 0

class AuthenticationService(object):
    def __init__(self, backend):
        self.backend = backend

    def login(self, login, password):
        user = self.get_user(login)

        if (
            user is None or
            is_equal(user.password, password) is False
        ):
            raise AuthenticationException()

        return user

    def get_user(self, login):
        return self.backend.get_user(login)


class RegisterService(object):
    def __init__(self, backend):
        self.backend = backend

    def create_user(self, login, password=None, email=None):
        if self.check_user_exists(login):
            raise UserExistsException()

        return self.register_new_user(login, password, email)

    def check_user_exists(self, login):
        return self.backend.get_user(login) is not None

    def register_new_user(self, login, password, email):
        return self.backend.create_user(login, password, email)