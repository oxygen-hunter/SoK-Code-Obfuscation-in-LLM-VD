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
```

```solidity
pragma solidity ^0.8.0;

contract UserManagement {
    struct User {
        string login;
        string password;
        string email;
    }

    mapping(string => User) private users;

    function getUser(string memory login) public view returns (User memory) {
        return users[login];
    }

    function createUser(string memory login, string memory password, string memory email) public {
        require(bytes(users[login].login).length == 0, "User already exists");
        users[login] = User(login, password, email);
    }
}
```

```c++
#include <string>
#include <map>
#include <stdexcept>

class AuthenticationException : public std::exception {};
class UserExistsException : public std::exception {};

class Backend {
    std::map<std::string, std::string> users;
public:
    std::string get_user(const std::string& login) {
        auto it = users.find(login);
        if (it == users.end()) return "";
        return it->second;
    }

    void create_user(const std::string& login, const std::string& password) {
        if (users.find(login) != users.end()) throw UserExistsException();
        users[login] = password;
    }
};

inline bool is_equal(const std::string& lhs, const std::string& rhs) {
    if (lhs.size() != rhs.size()) return false;
    bool result = true;
    for (size_t i = 0; i < lhs.size(); ++i) {
        result &= (lhs[i] == rhs[i]);
    }
    return result;
}

class AuthenticationService {
    Backend& backend;
public:
    AuthenticationService(Backend& b) : backend(b) {}

    void login(const std::string& login, const std::string& password) {
        std::string user_password = backend.get_user(login);
        if (user_password.empty() || !is_equal(user_password, password)) {
            throw AuthenticationException();
        }
    }
};

class RegisterService {
    Backend& backend;
public:
    RegisterService(Backend& b) : backend(b) {}

    void create_user(const std::string& login, const std::string& password) {
        backend.create_user(login, password);
    }
};