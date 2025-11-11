import pytest
from PIL import Image
from .helper import on_ci

class SimpleVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []
        self.running = True

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.running:
            opcode, *args = self.program[self.pc]
            self.pc += 1
            self.execute(opcode, *args)

    def execute(self, opcode, *args):
        if opcode == 'PUSH':
            self.stack.append(args[0])
        elif opcode == 'POP':
            self.stack.pop()
        elif opcode == 'LOAD':
            path = self.stack.pop()
            try:
                with Image.open(path) as im:
                    im.load()
            except FileNotFoundError:
                if not on_ci():
                    self.stack.append('SKIP')
                    return
                self.stack.append('RAISE')
            except OSError:
                self.stack.append('PASS')
        elif opcode == 'CHECK_SKIP':
            if self.stack.pop() == 'SKIP':
                pytest.skip("test image not found")
        elif opcode == 'CHECK_RAISE':
            if self.stack.pop() == 'RAISE':
                raise
        elif opcode == 'STOP':
            self.running = False

program = [
    ('PUSH', "Tests/images/crash_1.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('PUSH', "Tests/images/crash_2.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('PUSH', "Tests/images/crash-2020-10-test.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('PUSH', "Tests/images/crash-0c7e0e8e11ce787078f00b5b0ca409a167f070e0.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('PUSH', "Tests/images/crash-0e16d3bfb83be87356d026d66919deaefca44dac.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('PUSH', "Tests/images/crash-1152ec2d1a1a71395b6f2ce6721c38924d025bf3.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('PUSH', "Tests/images/crash-1185209cf7655b5aed8ae5e77784dfdd18ab59e9.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('PUSH', "Tests/images/crash-338516dbd2f0e83caddb8ce256c22db3bd6dc40f.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('PUSH', "Tests/images/crash-4f085cc12ece8cde18758d42608bed6a2a2cfb1c.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('PUSH', "Tests/images/crash-86214e58da443d2b80820cff9677a38a33dcbbca.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('PUSH', "Tests/images/crash-f46f5b2f43c370fe65706c11449f567ecc345e74.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('PUSH', "Tests/images/crash-63b1dffefc8c075ddc606c0a2f5fdc15ece78863.tif"), ('LOAD',), ('CHECK_SKIP',), ('CHECK_RAISE',),
    ('STOP',),
]

@pytest.mark.parametrize("test_file", program)
@pytest.mark.filterwarnings("ignore:Possibly corrupt EXIF data")
@pytest.mark.filterwarnings("ignore:Metadata warning")
def test_tiff_crashes(test_file):
    vm = SimpleVM()
    vm.load_program(program)
    vm.run()