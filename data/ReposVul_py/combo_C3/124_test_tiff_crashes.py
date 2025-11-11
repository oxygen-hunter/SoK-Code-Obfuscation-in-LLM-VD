import pytest
from PIL import Image
from .helper import on_ci

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.memory = {}

    def run(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            op = instr[0]
            args = instr[1:]
            getattr(self, f"op_{op}")(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_CALL(self, fn):
        fn()

    def op_LOAD(self, index):
        self.stack.append(self.memory.get(index, None))

    def op_STORE(self, index):
        self.memory[index] = self.stack.pop()

    def op_JMP(self, target):
        self.pc = target - 1

    def op_JZ(self, target):
        if not self.stack.pop():
            self.pc = target - 1

def test_tiff_crashes_vm():
    vm = VM()

    def load_image():
        test_file = vm.op_POP()
        try:
            with Image.open(test_file) as im:
                im.load()
        except FileNotFoundError:
            if not on_ci():
                pytest.skip("test image not found")
                return
            raise
        except OSError:
            pass

    test_files = [
        "Tests/images/crash_1.tif",
        "Tests/images/crash_2.tif",
        "Tests/images/crash-2020-10-test.tif",
        "Tests/images/crash-0c7e0e8e11ce787078f00b5b0ca409a167f070e0.tif",
        "Tests/images/crash-0e16d3bfb83be87356d026d66919deaefca44dac.tif",
        "Tests/images/crash-1152ec2d1a1a71395b6f2ce6721c38924d025bf3.tif",
        "Tests/images/crash-1185209cf7655b5aed8ae5e77784dfdd18ab59e9.tif",
        "Tests/images/crash-338516dbd2f0e83caddb8ce256c22db3bd6dc40f.tif",
        "Tests/images/crash-4f085cc12ece8cde18758d42608bed6a2a2cfb1c.tif",
        "Tests/images/crash-86214e58da443d2b80820cff9677a38a33dcbbca.tif",
        "Tests/images/crash-f46f5b2f43c370fe65706c11449f567ecc345e74.tif",
    ]

    instructions = []
    for test_file in test_files:
        instructions.extend([
            ("PUSH", test_file),
            ("CALL", load_image)
        ])

    vm.run(instructions)

@pytest.mark.parametrize("test_file", [
    "Tests/images/crash_1.tif",
    "Tests/images/crash_2.tif",
    "Tests/images/crash-2020-10-test.tif",
    "Tests/images/crash-0c7e0e8e11ce787078f00b5b0ca409a167f070e0.tif",
    "Tests/images/crash-0e16d3bfb83be87356d026d66919deaefca44dac.tif",
    "Tests/images/crash-1152ec2d1a1a71395b6f2ce6721c38924d025bf3.tif",
    "Tests/images/crash-1185209cf7655b5aed8ae5e77784dfdd18ab59e9.tif",
    "Tests/images/crash-338516dbd2f0e83caddb8ce256c22db3bd6dc40f.tif",
    "Tests/images/crash-4f085cc12ece8cde18758d42608bed6a2a2cfb1c.tif",
    "Tests/images/crash-86214e58da443d2b80820cff9677a38a33dcbbca.tif",
    "Tests/images/crash-f46f5b2f43c370fe65706c11449f567ecc345e74.tif",
])
@pytest.mark.filterwarnings("ignore:Possibly corrupt EXIF data")
@pytest.mark.filterwarnings("ignore:Metadata warning")
def test_tiff_crashes(test_file):
    test_tiff_crashes_vm()