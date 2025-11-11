from helper import unittest, PillowTestCase
from PIL import Image

class SimpleVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.memory = {}

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            op = self.instructions[self.pc]
            self.pc += 1
            if op == 'PUSH':
                value = self.instructions[self.pc]
                self.pc += 1
                self.stack.append(value)
            elif op == 'POP':
                self.stack.pop()
            elif op == 'LOAD':
                addr = self.stack.pop()
                self.stack.append(self.memory[addr])
            elif op == 'STORE':
                addr = self.stack.pop()
                self.memory[addr] = self.stack.pop()
            elif op == 'CALL':
                function = self.instructions[self.pc]
                self.pc += 1
                function()
            elif op == 'JMP':
                self.pc = self.instructions[self.pc]
            elif op == 'JZ':
                target = self.instructions[self.pc]
                self.pc += 1
                if self.stack.pop() == 0:
                    self.pc = target
            elif op == 'ADD':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif op == 'SUB':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)

class TestFilePcd(PillowTestCase):
    
    def test_load_raw(self):
        vm = SimpleVM()
        def load_image():
            im = Image.open('Tests/images/hopper.pcd')
            vm.stack.append(im)
        
        def load_image_method():
            im = vm.stack.pop()
            im.load()
            vm.stack.append(im)

        vm.load_instructions([
            'CALL', load_image,
            'CALL', load_image_method,
            'POP'
        ])
        
        vm.run()