from helper import unittest, PillowTestCase
from PIL import Image

TEST_FILE = "Tests/images/libtiff_segfault.tif"

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.pc += 1
            if instr[0] == 'PUSH':
                self.stack.append(instr[1])
            elif instr[0] == 'POP':
                self.stack.pop()
            elif instr[0] == 'CALL_FUNCTION':
                func = self.stack.pop()
                func()
            elif instr[0] == 'LOAD_CONST':
                index = instr[1]
                self.stack.append(self.constants[index])
            elif instr[0] == 'RETURN_VALUE':
                return self.stack.pop()
            elif instr[0] == 'TRY':
                try_block, except_block = instr[1], instr[2]
                try:
                    self.load_instructions(try_block)
                    self.run()
                except IOError:
                    self.load_instructions(except_block)
                    self.run()
            elif instr[0] == 'ASSERT_TRUE':
                expr, message = instr[1], instr[2]
                assert expr, message
            elif instr[0] == 'FAIL':
                message = instr[1]
                raise Exception(message)
            else:
                raise Exception("Unknown instruction")

class TestLibtiffSegfault(PillowTestCase):
    def test_segfault(self):
        vm = VirtualMachine()
        instructions = [
            ('TRY', [
                ('PUSH', lambda: setattr(self, 'im', Image.open(TEST_FILE))),
                ('CALL_FUNCTION',),
                ('PUSH', lambda: self.im.load()),
                ('CALL_FUNCTION',),
                ('RETURN_VALUE',),
            ], [
                ('PUSH', True),
                ('PUSH', "Got expected IOError"),
                ('ASSERT_TRUE',),
                ('RETURN_VALUE',),
            ]),
            ('PUSH', "Should have returned IOError"),
            ('FAIL',)
        ]
        vm.load_instructions(instructions)
        vm.run()

if __name__ == '__main__':
    unittest.main()