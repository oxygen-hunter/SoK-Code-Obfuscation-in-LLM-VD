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
        user = self.backend.get_user(login)

        dispatcher = 0
        while True:
            if dispatcher == 0:
                if user is None:
                    dispatcher = 1
                else:
                    dispatcher = 2
            elif dispatcher == 1:
                raise AuthenticationException()
            elif dispatcher == 2:
                if is_equal(user.password, password) is False:
                    dispatcher = 1
                else:
                    dispatcher = 3
            elif dispatcher == 3:
                return user

class RegisterService(object):
    def __init__(self, backend):
        self.backend = backend

    def create_user(self, login, password=None, email=None):
        user = self.backend.get_user(login)

        dispatcher = 0
        while True:
            if dispatcher == 0:
                if user is not None:
                    dispatcher = 1
                else:
                    dispatcher = 2
            elif dispatcher == 1:
                raise UserExistsException()
            elif dispatcher == 2:
                user = self.backend.create_user(
                    login,
                    password,
                    email
                )
                dispatcher = 3
            elif dispatcher == 3:
                return user