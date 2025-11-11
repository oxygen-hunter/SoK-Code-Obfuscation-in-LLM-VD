from helper import unittest, PillowTestCase
from PIL import Image

class VM:
    def __init__(self, instructions):
        self.instructions = instructions
        self.stack = []
        self.pc = 0
        self.running = True

    def run(self):
        while self.running:
            op = self.instructions[self.pc]
            self.dispatch(op)
            self.pc += 1

    def dispatch(self, op):
        if op[0] == 'PUSH':
            self.stack.append(op[1])
        elif op[0] == 'LOAD':
            self.stack.append(Image.open(self.stack.pop()))
        elif op[0] == 'CALL':
            getattr(self.stack[-1], op[1])()
        elif op[0] == 'JMP':
            self.pc = op[1] - 1
        elif op[0] == 'HALT':
            self.running = False

class TestFliOverflow(PillowTestCase):
    def test_fli_overflow(self):
        instructions = [
            ('PUSH', "Tests/images/fli_overflow.fli"),
            ('LOAD',),
            ('CALL', 'load'),
            ('HALT',)
        ]
        vm = VM(instructions)
        vm.run()

if __name__ == '__main__':
    unittest.main()