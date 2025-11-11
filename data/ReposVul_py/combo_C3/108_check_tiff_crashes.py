#!/usr/bin/env python

from PIL import Image

# Define the virtual machine class
class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True

    def load_program(self, program):
        self.instructions = program

    def run(self):
        while self.running:
            inst = self.instructions[self.pc]
            self.pc += 1
            self.execute(inst)

    def execute(self, inst):
        if inst[0] == 'PUSH':
            self.stack.append(inst[1])
        elif inst[0] == 'POP':
            self.stack.pop()
        elif inst[0] == 'LOAD':
            path = self.stack.pop()
            try:
                im = Image.open(path)
                im.load()
            except Exception as msg:
                print(msg)
        elif inst[0] == 'JMP':
            self.pc = inst[1]
        elif inst[0] == 'HALT':
            self.running = False
        else:
            raise Exception("Unknown instruction {}".format(inst))

# Compiled sequence of instructions
program = [
    ('PUSH', "images/crash_1.tif"),
    ('LOAD',),
    ('POP',),
    ('PUSH', "images/crash_2.tif"),
    ('LOAD',),
    ('POP',),
    ('HALT',)
]

# Create a virtual machine and execute the program
vm = VM()
vm.load_program(program)
vm.run()