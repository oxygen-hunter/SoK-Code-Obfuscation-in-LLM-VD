from helper import unittest, PillowTestCase, hopper
from PIL import Image

class VM:
    def __init__(self):
        self.stack = []
        self.program_counter = 0
        self.instructions = {
            'PUSH': self.push,
            'POP': self.pop,
            'LOAD_IMAGE': self.load_image,
            'LOAD': lambda: self.stack[-1].load(),
            'JUMP_IF_TRUE': self.jump_if_true,
            'HALT': lambda: setattr(self, 'halted', True)
        }
        self.halted = False

    def execute(self, bytecode):
        self.halted = False
        while not self.halted:
            instruction = bytecode[self.program_counter]
            self.program_counter += 1
            self.instructions[instruction[0]](*instruction[1:])

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        return self.stack.pop()

    def load_image(self, path):
        self.stack.append(Image.open(path))

    def jump_if_true(self, target):
        if self.stack.pop():
            self.program_counter = target

class TestFilePcd(PillowTestCase):
    def test_load_raw(self):
        vm = VM()
        bytecode = [
            ('LOAD_IMAGE', 'Tests/images/hopper.pcd'),
            ('LOAD',),
            ('PUSH', True),  # simulate a true condition
            ('JUMP_IF_TRUE', 3),
            ('HALT',)
        ]
        vm.execute(bytecode)
        # target = hopper().resize((768, 512))
        # self.assert_image_similar(vm.stack[-1], target, 10)

if __name__ == '__main__':
    unittest.main()