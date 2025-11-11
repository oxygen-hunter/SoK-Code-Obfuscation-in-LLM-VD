# -*- coding: utf-8 -*-

import random
import string
import js2py

js2py.disable_pyimport()

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.registers = {}

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            op, *args = self.instructions[self.pc]
            getattr(self, f"op_{op}")(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        self.stack.pop()

    def op_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, target):
        self.pc = target - 1

    def op_JZ(self, target):
        if self.stack.pop() == 0:
            self.pc = target - 1

    def op_LOAD(self, var_name):
        self.stack.append(self.registers[var_name])

    def op_STORE(self, var_name):
        self.registers[var_name] = self.stack.pop()

# Function implementations as VM instruction sequences
def random_string_vm(length):
    vm = VM()
    seq = string.ascii_letters + string.digits + string.punctuation
    vm.load_instructions([
        ("PUSH", ''),
        ("STORE", 'result'),
        ("PUSH", seq),
        ("STORE", 'seq'),
        ("PUSH", length),
        ("STORE", 'length'),
        ("PUSH", 0),
        ("STORE", 'i'),
        ("LOAD", 'i'),
        ("LOAD", 'length'),
        ("SUB",),
        ("JZ", 18),
        ("LOAD", 'result'),
        ("LOAD", 'seq'),
        ("PUSH", random.choice),
        ("ADD",),
        ("STORE", 'result'),
        ("LOAD", 'i'),
        ("PUSH", 1),
        ("ADD",),
        ("STORE", 'i'),
        ("JMP", 6),
        ("LOAD", 'result')
    ])
    vm.run()
    return vm.stack.pop()

def is_plural_vm(value):
    vm = VM()
    vm.load_instructions([
        ("PUSH", value),
        ("STORE", 'value'),
        ("PUSH", 0),
        ("STORE", 'result'),
        ("PUSH", 0),
        ("STORE", 'n'),
        ("LOAD", 'value'),
        ("PUSH", float),
        ("ADD",),
        ("LOAD", 'n'),
        ("LOAD", 'result'),
        ("ADD",),
        ("STORE", 'n'),
        ("LOAD", 'n'),
        ("LOAD", 0),
        ("SUB",),
        ("JZ", 12),
        ("LOAD", 'n'),
        ("LOAD", 1),
        ("SUB",),
        ("JZ", 12),
        ("PUSH", 1),
        ("STORE", 'result'),
        ("LOAD", 'result')
    ])
    try:
        vm.run()
        return vm.stack.pop() == 1
    except ValueError:
        return value.endswith("s")

def eval_js_vm(script, es6=False):
    vm = VM()
    vm.load_instructions([
        ("PUSH", es6),
        ("JZ", 4),
        ("PUSH", js2py.eval_js6),
        ("JMP", 5),
        ("PUSH", js2py.eval_js),
        ("LOAD", 'script'),
        ("ADD",)
    ])
    vm.run()
    return vm.stack.pop()(script)

def accumulate_vm(iterable, to_map=None):
    vm = VM()
    vm.load_instructions([
        ("PUSH", to_map),
        ("STORE", 'to_map'),
        ("PUSH", None),
        ("STORE", 'result'),
        ("PUSH", 0),
        ("STORE", 'empty'),
        ("LOAD", 'to_map'),
        ("LOAD", 'empty'),
        ("JZ", 5),
        ("PUSH", {}),
        ("STORE", 'to_map'),
        ("LOAD", 'iterable'),
        ("STORE", 'iterable'),
        ("LOAD", 'to_map'),
        ("STORE", 'to_map'),
        ("LOAD", 'result')
    ])
    vm.run()
    to_map = vm.stack.pop()
    for key, value in iterable:
        to_map.setdefault(value, []).append(key)
    return to_map

def reversemap_vm(obj):
    vm = VM()
    vm.load_instructions([
        ("PUSH", obj),
        ("STORE", 'obj'),
        ("LOAD", 'obj'),
        ("PUSH", obj.__class__),
        ("ADD",),
        ("STORE", 'result'),
        ("LOAD", 'result')
    ])
    vm.run()
    return vm.stack.pop()(reversed(item) for item in obj.items())