import unittest

from vncap.vnc.protocol import VNCServerAuthenticator

class DummyTransport(object):

    def __init__(self):
        self.buf = ""
        self.lost = False

    def write(self, data):
        self.buf += data

    def loseConnection(self):
        self.lost = True

    def pauseProducing(self):
        pass

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        return self.stack.pop()

    def execute(self, program):
        self.pc = 0
        self.program = program
        while self.pc < len(self.program):
            instruction, *args = self.program[self.pc]
            getattr(self, instruction)(*args)
            self.pc += 1

    def LOAD(self, value):
        self.push(value)

    def STORE(self, var):
        setattr(self, var, self.pop())

    def ADD(self):
        b = self.pop()
        a = self.pop()
        self.push(a + b)

    def SUB(self):
        b = self.pop()
        a = self.pop()
        self.push(a - b)

    def JMP(self, target):
        self.pc = target - 1

    def JZ(self, target):
        if self.pop() == 0:
            self.pc = target - 1

    def CALL(self, func):
        func()

class TestVNCServerAuthenticator(unittest.TestCase):

    def setUp(self):
        self.p = VNCServerAuthenticator("password", {})
        self.t = DummyTransport()
        self.vm = VM()
        self.vm.execute([
            ('CALL', self.p.makeConnection),
            ('CALL', self.t)
        ])

    def test_trivial(self):
        pass

    def test_connectionMade(self):
        self.vm.execute([
            ('LOAD', "RFB 003.008\n"),
            ('STORE', 'expected'),
            ('LOAD', self.t.buf),
            ('LOAD', 'expected'),
            ('SUB',),
            ('JZ', 2)
        ])
        self.assertEqual(self.t.buf, "RFB 003.008\n")

    def test_check_version(self):
        self.vm.execute([
            ('LOAD', ""),
            ('STORE', 'self.t.buf'),
            ('CALL', lambda: self.p.check_version("RFB 003.008\n")),
            ('LOAD', "\x01\x02"),
            ('STORE', 'expected'),
            ('LOAD', self.t.buf),
            ('LOAD', 'expected'),
            ('SUB',),
            ('JZ', 2)
        ])
        self.assertEqual(self.t.buf, "\x01\x02")

    def test_check_invalid_version(self):
        self.vm.execute([
            ('LOAD', ""),
            ('STORE', 'self.t.buf'),
            ('CALL', lambda: self.p.check_version("RFB 002.000\n")),
            ('LOAD', True),
            ('STORE', 'expected'),
            ('LOAD', self.t.lost),
            ('LOAD', 'expected'),
            ('SUB',),
            ('JZ', 2)
        ])
        self.assertTrue(self.t.lost)

    def test_select_security_type_none(self):
        self.vm.execute([
            ('LOAD', ""),
            ('STORE', 'self.t.buf'),
            ('CALL', lambda: self.p.select_security_type("\x01")),
            ('LOAD', True),
            ('STORE', 'expected'),
            ('LOAD', self.t.lost),
            ('LOAD', 'expected'),
            ('SUB',),
            ('JZ', 2)
        ])
        self.assertTrue(self.t.lost)

    def test_select_security_type_vnc_auth(self):
        self.vm.execute([
            ('LOAD', ""),
            ('STORE', 'self.t.buf'),
            ('CALL', lambda: self.p.select_security_type("\x02")),
            ('LOAD', False),
            ('STORE', 'expected_lost'),
            ('LOAD', self.t.lost),
            ('LOAD', 'expected_lost'),
            ('SUB',),
            ('JZ', 2),
            ('LOAD', 16),
            ('STORE', 'expected_len'),
            ('LOAD', len(self.t.buf)),
            ('LOAD', 'expected_len'),
            ('SUB',),
            ('JZ', 2)
        ])
        self.assertFalse(self.t.lost)
        self.assertEqual(len(self.t.buf), 16)