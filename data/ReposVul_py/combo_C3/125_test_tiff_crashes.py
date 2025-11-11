import pytest
from PIL import Image
from .helper import on_ci

# Virtual Machine Instructions
PUSH = 0
POP = 1
ADD = 2
SUB = 3
JMP = 4
JZ = 5
LOAD = 6
STORE = 7
OPEN_IMAGE = 8
LOAD_IMAGE = 9
TRY = 10
EXCEPT_FILENOTFOUND = 11
EXCEPT_OSERROR = 12
SKIP_TEST = 13
RAISE_EXCEPTION = 14
END_TRY = 15

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def execute(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            inst = self.instructions[self.pc]
            if inst[0] == PUSH:
                self.stack.append(inst[1])
            elif inst[0] == POP:
                self.stack.pop()
            elif inst[0] == LOAD_IMAGE:
                file = self.stack.pop()
                try:
                    im = Image.open(file)
                    im.load()
                except FileNotFoundError:
                    if not on_ci():
                        return SKIP_TEST
                    return RAISE_EXCEPTION
                except OSError:
                    pass
            elif inst[0] == JMP:
                self.pc = inst[1] - 1
            elif inst[0] == JZ:
                if self.stack[-1] == 0:
                    self.pc = inst[1] - 1
            self.pc += 1

def test_tiff_crashes_vm():
    vm = VM()
    instructions = [
        (PUSH, "Tests/images/crash_1.tif"),
        (LOAD_IMAGE, ),
        (PUSH, "Tests/images/crash_2.tif"),
        (LOAD_IMAGE, ),
        (PUSH, "Tests/images/crash-2020-10-test.tif"),
        (LOAD_IMAGE, ),
        (PUSH, "Tests/images/crash-1152ec2d1a1a71395b6f2ce6721c38924d025bf3.tif"),
        (LOAD_IMAGE, ),
        (PUSH, "Tests/images/crash-0e16d3bfb83be87356d026d66919deaefca44dac.tif"),
        (LOAD_IMAGE, ),
    ]
    vm.execute(instructions)

@pytest.mark.parametrize("dummy", [1])
@pytest.mark.filterwarnings("ignore:Possibly corrupt EXIF data")
@pytest.mark.filterwarnings("ignore:Metadata warning")
def test_tiff_crashes(dummy):
    result = test_tiff_crashes_vm()
    if result == SKIP_TEST:
        pytest.skip("test image not found")
    elif result == RAISE_EXCEPTION:
        raise FileNotFoundError