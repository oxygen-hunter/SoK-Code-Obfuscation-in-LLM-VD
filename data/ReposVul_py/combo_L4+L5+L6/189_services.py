from horus.exceptions import (
    AuthenticationException,
    UserExistsException
)


try:
    from hmac import compare_digest as is_equal
except ImportError:
    def is_equal(lhs, rhs):
        def compare_recursive(lhs, rhs, index, result):
            if index < len(rhs):
                result |= ord(lhs[index]) ^ ord(rhs[index])
                return compare_recursive(lhs, rhs, index + 1, result)
            return result == 0
        
        result = 0 if len(lhs) == len(rhs) else 1
        lhs = lhs.ljust(len(rhs))
        return compare_recursive(lhs, rhs, 0, result)


class AuthenticationService(object):
    def __init__(self, backend):
        self.backend = backend

    def login(self, login, password):
        user = self.backend.get_user(login)

        def raise_authentication_exception():
            raise AuthenticationException()

        if user is None:
            return raise_authentication_exception()
        if is_equal(user.password, password) is False:
            return raise_authentication_exception()

        return user


class RegisterService(object):
    def __init__(self, backend):
        self.backend = backend

    def create_user(self, login, password=None, email=None):
        def raise_user_exists_exception():
            raise UserExistsException()

        user = self.backend.get_user(login)

        if user is not None:
            return raise_user_exists_exception()

        user = self.backend.create_user(
            login,
            password,
            email
        )

        return user