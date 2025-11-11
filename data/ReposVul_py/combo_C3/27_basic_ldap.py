import sys
import ldap  # pylint: disable=import-error
from flask import current_app, jsonify, request
from flask_cors import cross_origin
from alerta.auth.utils import create_token, get_customers
from alerta.exceptions import ApiError
from alerta.models.permission import Permission
from alerta.models.user import User
from alerta.utils.audit import auth_audit_trail
from . import auth

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.running = True
        self.instructions = {
            "PUSH": self.push,
            "POP": self.pop,
            "ADD": self.add,
            "STORE": self.store,
            "LOAD": self.load,
            "JMP": self.jmp,
            "JZ": self.jz,
            "CALL": self.call,
            "RET": self.ret
        }
        self.memory = {}
        self.program = []

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        return self.stack.pop()

    def add(self):
        b = self.pop()
        a = self.pop()
        self.push(a + b)

    def store(self, address):
        value = self.pop()
        self.memory[address] = value

    def load(self, address):
        self.push(self.memory.get(address, 0))

    def jmp(self, address):
        self.pc = address - 1

    def jz(self, address):
        if self.pop() == 0:
            self.pc = address - 1

    def call(self, address):
        self.push(self.pc + 1)
        self.pc = address - 1

    def ret(self):
        self.pc = self.pop() - 1

    def run(self):
        while self.running and self.pc < len(self.program):
            instruction = self.program[self.pc]
            op_code = instruction[0]
            args = instruction[1:]
            self.instructions[op_code](*args)
            self.pc += 1

    def load_program(self, program):
        self.program = program

def compile_login():
    vm = VM()
    program = [
        ("PUSH", current_app.config['LDAP_ALLOW_SELF_SIGNED_CERT']),
        ("CALL", 2),
        ("PUSH", request.json.get('username', None)),
        ("PUSH", request.json['email']),
        ("PUSH", request.json['password']),
        ("CALL", 20),
        ("PUSH", '\\'),
        ("CALL", 34),
        ("PUSH", '@'),
        ("CALL", 49),
        ("PUSH", domain),
        ("PUSH", current_app.config['LDAP_DOMAINS']),
        ("CALL", 65),
        ("PUSH", current_app.config['LDAP_DOMAINS'][domain] % username),
        ("CALL", 79),
        ("CALL", 84),
        ("PUSH", not email_verified),
        ("CALL", 100),
        ("PUSH", login),
        ("CALL", 120),
        ("PUSH", user),
        ("CALL", 135),
        ("PUSH", ldap_connection),
        ("CALL", 144),
        ("PUSH", user.status != 'active'),
        ("CALL", 155),
        ("PUSH", user),
        ("CALL", 164),
        ("PUSH", login),
        ("PUSH", user.roles + groups),
        ("CALL", 177),
        ("PUSH", login),
        ("PUSH", [user.domain] + groups),
        ("CALL", 188),
        ("PUSH", current_app._get_current_object()),
        ("PUSH", 'basic-ldap-login'),
        ("PUSH", 'user login via LDAP'),
        ("PUSH", login),
        ("PUSH", customers),
        ("PUSH", scopes),
        ("PUSH", user.roles),
        ("PUSH", groups),
        ("PUSH", user.id),
        ("PUSH", 'user'),
        ("PUSH", request),
        ("CALL", 199),
        ("PUSH", user.id),
        ("PUSH", user.name),
        ("PUSH", user.email),
        ("PUSH", 'ldap'),
        ("PUSH", customers),
        ("PUSH", scopes),
        ("PUSH", user.roles),
        ("PUSH", groups),
        ("PUSH", user.email),
        ("PUSH", user.email_verified),
        ("CALL", 225),
        ("PUSH", token.tokenize),
        ("RET",)
    ]
    vm.load_program(program)
    vm.run()

@auth.route('/auth/login', methods=['OPTIONS', 'POST'])
@cross_origin(supports_credentials=True)
def login():
    compile_login()
    return jsonify(token=compile_login())