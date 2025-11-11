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

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.memory = {}
    
    def run(self, bytecode):
        self.instructions = bytecode
        while self.pc < len(self.instructions):
            self.dispatch()
    
    def dispatch(self):
        instruction = self.instructions[self.pc]
        op = instruction[0]
        args = instruction[1:]
        
        if op == "PUSH":
            self.stack.append(args[0])
        elif op == "POP":
            self.stack.pop()
        elif op == "ADD":
            self.stack.append(self.stack.pop() + self.stack.pop())
        elif op == "SUB":
            self.stack.append(self.stack.pop() - self.stack.pop())
        elif op == "JMP":
            self.pc = args[0] - 1
        elif op == "JZ":
            if self.stack.pop() == 0:
                self.pc = args[0] - 1
        elif op == "LOAD":
            self.stack.append(self.memory[args[0]])
        elif op == "STORE":
            self.memory[args[0]] = self.stack.pop()
        elif op == "ISEQUAL":
            rhs = self.stack.pop()
            lhs = self.stack.pop()
            self.stack.append(1 if is_equal(lhs, rhs) else 0)
        elif op == "RAISE_AUTH":
            raise AuthenticationException()
        elif op == "RAISE_USER_EXISTS":
            raise UserExistsException()

        self.pc += 1

class AuthenticationService:
    def __init__(self, backend):
        self.backend = backend

    def login(self, login, password):
        vm = VM()
        bytecode = [
            ("PUSH", self.backend.get_user(login)),
            ("STORE", "user"),
            ("LOAD", "user"),
            ("PUSH", None),
            ("ISEQUAL",),
            ("JZ", 10),
            ("LOAD", "user"),
            ("PUSH", "password"),
            ("PUSH", password),
            ("ISEQUAL",),
            ("JZ", 10),
            ("LOAD", "user"),
            ("JMP", 11),
            ("RAISE_AUTH",)
        ]
        vm.run(bytecode)
        return vm.stack[-1]

class RegisterService:
    def __init__(self, backend):
        self.backend = backend

    def create_user(self, login, password=None, email=None):
        vm = VM()
        bytecode = [
            ("PUSH", self.backend.get_user(login)),
            ("STORE", "user"),
            ("LOAD", "user"),
            ("PUSH", None),
            ("ISEQUAL",),
            ("JZ", 6),
            ("RAISE_USER_EXISTS",),
            ("PUSH", self.backend.create_user(login, password, email)),
            ("STORE", "new_user"),
            ("LOAD", "new_user")
        ]
        vm.run(bytecode)
        return vm.stack[-1]