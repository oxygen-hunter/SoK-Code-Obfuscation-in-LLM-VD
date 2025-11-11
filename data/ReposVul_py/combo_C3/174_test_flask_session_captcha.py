import unittest
from flask_session_captcha import FlaskSessionCaptcha
from flask import Flask, request
from flask_sessionstore import Session

class SimpleVM:
    def __init__(self):
        self.stack = []
        self.memory = {}
        self.pc = 0
        self.running = True

    def run(self, program):
        self.pc = 0
        self.running = True
        self.program = program
        while self.running:
            instr = self.program[self.pc]
            self.execute(instr)

    def execute(self, instr):
        op = instr[0]
        if op == 'PUSH':
            self.stack.append(instr[1])
        elif op == 'POP':
            self.stack.pop()
        elif op == 'ADD':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a + b)
        elif op == 'SUB':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a - b)
        elif op == 'LOAD':
            self.stack.append(self.memory[instr[1]])
        elif op == 'STORE':
            self.memory[instr[1]] = self.stack.pop()
        elif op == 'JMP':
            self.pc = instr[1] - 1
        elif op == 'JZ':
            if self.stack.pop() == 0:
                self.pc = instr[1] - 1
        elif op == 'STOP':
            self.running = False
        self.pc += 1

def setup_vm(app):
    vm = SimpleVM()
    program = [
        ('PUSH', 'SECRET_KEY'),
        ('PUSH', 'aba'),
        ('STORE', app),
        ('PUSH', 'SQLALCHEMY_DATABASE_URI'),
        ('PUSH', 'sqlite://'),
        ('STORE', app),
        ('PUSH', 'SQLALCHEMY_TRACK_MODIFICATIONS'),
        ('PUSH', False),
        ('STORE', app),
        ('PUSH', 'SESSION_TYPE'),
        ('PUSH', 'sqlalchemy'),
        ('STORE', app),
        ('PUSH', 'CAPTCHA_ENABLE'),
        ('PUSH', True),
        ('STORE', app),
        ('PUSH', 'CAPTCHA_LENGTH'),
        ('PUSH', 5),
        ('STORE', app),
        ('STOP',)
    ]
    vm.run(program)

class FlaskSessionCaptchaTestCase(unittest.TestCase):
    def setUp(self):
        self.app = Flask(__name__)
        setup_vm(self.app.config)
        self.app.testing = True
        Session(self.app)
        self.client = self.app.test_client()

    def test_captcha_wrong(self):
        self.run_vm_test([
            ('PUSH', FlaskSessionCaptcha(self.app)),
            ('PUSH', _default_routes),
            ('PUSH', self.app),
            ('CALL', 2),
            ('PUSH', self.client),
            ('PUSH', "/"),
            ('PUSH', {"s": "something"}),
            ('CALL', 2, 'POST'),
            ('PUSH', 'nope'),
            ('EQ',),
            ('ASSERT',),
            ('PUSH', self.client),
            ('PUSH', "/"),
            ('PUSH', {"s": "something", "captcha": ""}),
            ('CALL', 2, 'POST'),
            ('PUSH', 'nope'),
            ('EQ',),
            ('ASSERT',),
            ('PUSH', self.client),
            ('PUSH', "/"),
            ('PUSH', {"s": "something", "captcha": "also wrong"}),
            ('CALL', 2, 'POST'),
            ('PUSH', 'nope'),
            ('EQ',),
            ('ASSERT',),
        ])

    def run_vm_test(self, program):
        vm = SimpleVM()
        vm.run(program)

def _default_routes(captcha, app):
    @app.route("/", methods=["POST", "GET"])
    def hello():
        if request.method == "POST":
            if captcha.validate():
                return "ok"
            return "nope"
        captcha.generate()
        return str(captcha.get_answer())