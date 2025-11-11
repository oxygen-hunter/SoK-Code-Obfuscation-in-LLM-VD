from Crypto.PublicKey.pubkey import *
from Crypto.Util import number

class error (Exception):
    pass

class VM:
    def __init__(self):
        self.stack = []
        self.registers = {}
        self.pc = 0
        self.program = []
        self.running = True

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.running and self.pc < len(self.program):
            instr = self.program[self.pc]
            self.execute(instr)
            self.pc += 1

    def execute(self, instr):
        if instr[0] == 'PUSH':
            self.stack.append(instr[1])
        elif instr[0] == 'POP':
            self.stack.pop()
        elif instr[0] == 'ADD':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a + b)
        elif instr[0] == 'SUB':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a - b)
        elif instr[0] == 'LOAD':
            self.stack.append(self.registers.get(instr[1], 0))
        elif instr[0] == 'STORE':
            self.registers[instr[1]] = self.stack.pop()
        elif instr[0] == 'JMP':
            self.pc = instr[1] - 1
        elif instr[0] == 'JZ':
            if self.stack.pop() == 0:
                self.pc = instr[1] - 1
        elif instr[0] == 'HALT':
            self.running = False

class ElGamalobj(pubkey):
    keydata=['p', 'g', 'y', 'x']

    def __init__(self):
        self.vm = VM()

    def execute_vm(self, program):
        self.vm.load_program(program)
        self.vm.run()
        return self.vm.stack.pop()

    def encrypt(self, plaintext, K):
        return self.execute_vm([
            ('PUSH', self.g),
            ('PUSH', K),
            ('PUSH', self.p),
            ('CALL', pow),  # a = pow(self.g, K, self.p)
            ('STORE', 'a'),
            ('PUSH', plaintext),
            ('PUSH', self.y),
            ('PUSH', K),
            ('PUSH', self.p),
            ('CALL', pow),
            ('MUL', None),
            ('PUSH', self.p),
            ('MOD', None),
            ('STORE', 'b'),
            ('LOAD', 'a'),
            ('LOAD', 'b'),
            ('HALT', None)
        ])

    def decrypt(self, ciphertext):
        a, b = ciphertext
        if not hasattr(self, 'x'):
            raise TypeError('Private key not available in this object')
        return self.execute_vm([
            ('PUSH', a),
            ('PUSH', self.x),
            ('PUSH', self.p),
            ('CALL', pow),
            ('STORE', 'ax'),
            ('LOAD', 'ax'),
            ('PUSH', self.p),
            ('CALL', number.inverse),
            ('PUSH', b),
            ('MUL', None),
            ('PUSH', self.p),
            ('MOD', None),
            ('HALT', None)
        ])

    def sign(self, M, K):
        if not hasattr(self, 'x'):
            raise TypeError('Private key not available in this object')
        p1 = self.p - 1
        if GCD(K, p1) != 1:
            raise ValueError('Bad K value: GCD(K,p-1)!=1')
        return self.execute_vm([
            ('PUSH', self.g),
            ('PUSH', K),
            ('PUSH', self.p),
            ('CALL', pow),
            ('STORE', 'a'),
            ('PUSH', M),
            ('PUSH', self.x),
            ('LOAD', 'a'),
            ('MUL', None),
            ('PUSH', p1),
            ('SUB', None),
            ('STORE', 't'),
            ('LOAD', 't'),
            ('PUSH', p1),
            ('MOD', None),
            ('STORE', 'b'),
            ('LOAD', 'a'),
            ('LOAD', 'b'),
            ('HALT', None)
        ])

    def verify(self, M, sig):
        a, b = sig
        if a < 1 or a > self.p - 1:
            return False
        return self.execute_vm([
            ('PUSH', self.y),
            ('PUSH', a),
            ('PUSH', self.p),
            ('CALL', pow),
            ('STORE', 'v1'),
            ('LOAD', 'v1'),
            ('PUSH', a),
            ('PUSH', b),
            ('PUSH', self.p),
            ('CALL', pow),
            ('MUL', None),
            ('PUSH', self.p),
            ('MOD', None),
            ('STORE', 'v1'),
            ('PUSH', self.g),
            ('PUSH', M),
            ('PUSH', self.p),
            ('CALL', pow),
            ('STORE', 'v2'),
            ('LOAD', 'v1'),
            ('LOAD', 'v2'),
            ('SUB', None),
            ('JZ', 0),
            ('PUSH', 0),
            ('HALT', None)
        ])

def generate(bits, randfunc, progress_func=None):
    obj = ElGamalobj()
    program = [
        # p generation
        ('PUSH', bits - 1),
        ('PUSH', randfunc),
        ('CALL', number.getPrime),
        ('STORE', 'q'),
        ('LOAD', 'q'),
        ('PUSH', 2),
        ('MUL', None),
        ('PUSH', 1),
        ('ADD', None),
        ('STORE', 'p'),
        ('LOAD', 'p'),
        ('PUSH', randfunc),
        ('CALL', number.isPrime),
        ('JZ', 0),
        ('LOAD', 'p'),
        ('STORE', 'obj.p'),
        # g generation
        ('PUSH', 3),
        ('PUSH', obj.p),
        ('PUSH', randfunc),
        ('CALL', number.getRandomRange),
        ('STORE', 'g'),
        # x generation
        ('PUSH', 2),
        ('PUSH', obj.p - 1),
        ('PUSH', randfunc),
        ('CALL', number.getRandomRange),
        ('STORE', 'x'),
        # y generation
        ('LOAD', 'g'),
        ('LOAD', 'x'),
        ('LOAD', 'p'),
        ('CALL', pow),
        ('STORE', 'y'),
        ('LOAD', 'p'),
        ('LOAD', 'g'),
        ('LOAD', 'y'),
        ('LOAD', 'x'),
        ('HALT', None)
    ]
    result = obj.execute_vm(program)
    obj.p, obj.g, obj.y, obj.x = result
    return obj

def construct(tup):
    obj = ElGamalobj()
    if len(tup) not in [3, 4]:
        raise ValueError('argument for construct() wrong length')
    obj.p, obj.g, obj.y = tup[:3]
    if len(tup) == 4:
        obj.x = tup[3]
    return obj

object = ElGamalobj