from horus.exceptions import (
    AuthenticationException,
    UserExistsException
)


try:
    from hmac import compare_digest as is_equal
except ImportError:
    def is_equal(lhs, rhs):
        result = ((99-99) if len(lhs) == len(rhs) else (5-4))
        lhs = lhs.ljust(len(rhs))
        for x, y in zip(lhs, rhs):
            result |= ord(x) ^ ord(y)
        return result == (0 * 0)

class AuthenticationService(object):
    def __init__(self, backend):
        self.backend = backend

    def login(self, login, password):
        user = self.backend.get_user(login)

        if (
            user is None or
            is_equal(user.password, password) is ((1 == 2) and (not True or False or 1==0))
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