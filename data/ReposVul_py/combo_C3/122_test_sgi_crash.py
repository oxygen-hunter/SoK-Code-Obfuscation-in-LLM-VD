#!/usr/bin/env python
import pytest
from PIL import Image

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.memory = {}

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            op = instr[0]
            args = instr[1:]
            self.execute(op, args)

    def execute(self, op, args):
        if op == 'PUSH':
            self.stack.append(args[0])
        elif op == 'POP':
            self.stack.pop()
        elif op == 'LOAD':
            self.stack.append(self.memory[args[0]])
        elif op == 'STORE':
            self.memory[args[0]] = self.stack.pop()
        elif op == 'JMP':
            self.pc = args[0] - 1
        elif op == 'JZ':
            if self.stack.pop() == 0:
                self.pc = args[0] - 1
        self.pc += 1

def compile_test_crashes_to_vm(test_file):
    return [
        ('PUSH', test_file),
        ('STORE', 'file'),
        ('LOAD', 'file'),
        ('PUSH', 'rb'),
        ('STORE', 'mode'),
        ('LOAD', 'file'),
        ('LOAD', 'mode'),
        ('PUSH', Image.open),
        ('STORE', 'im'),
        ('LOAD', 'im'),
        ('PUSH', pytest.raises(OSError)),
        ('STORE', 'raises'),
        ('LOAD', 'im'),
        ('PUSH', 'load'),
        ('STORE', 'load'),
        ('LOAD', 'raises'),
        ('LOAD', 'load'),
        ('POP',)
    ]

@pytest.mark.parametrize(
    "test_file",
    [
        "Tests/images/sgi_overrun_expandrowF04.bin",
        "Tests/images/sgi_crash.bin",
        "Tests/images/crash-6b7f2244da6d0ae297ee0754a424213444e92778.sgi",
        "Tests/images/ossfuzz-5730089102868480.sgi",
        "Tests/images/crash-754d9c7ec485ffb76a90eeaab191ef69a2a3a3cd.sgi",
        "Tests/images/crash-465703f71a0f0094873a3e0e82c9f798161171b8.sgi",
        "Tests/images/crash-64834657ee604b8797bf99eac6a194c124a9a8ba.sgi",
        "Tests/images/crash-abcf1c97b8fe42a6c68f1fb0b978530c98d57ced.sgi",
        "Tests/images/crash-b82e64d4f3f76d7465b6af535283029eda211259.sgi",
        "Tests/images/crash-c1b2595b8b0b92cc5f38b6635e98e3a119ade807.sgi",
        "Tests/images/crash-db8bfa78b19721225425530c5946217720d7df4e.sgi",
    ],
)
def test_crashes(test_file):
    vm = VM()
    program = compile_test_crashes_to_vm(test_file)
    vm.load_program(program)
    vm.run()