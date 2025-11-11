import pytest
from PIL import FliImagePlugin, Image
from .helper import assert_image_equal_tofile, is_pypy

VM_STACK = []
VM_MEMORY = {}

class VM:
    def __init__(self, program):
        self.program = program
        self.pc = 0

    def run(self):
        while self.pc < len(self.program):
            opcode, *args = self.program[self.pc]
            self.execute(opcode, *args)
            self.pc += 1

    def execute(self, opcode, *args):
        if opcode == "PUSH":
            VM_STACK.append(args[0])
        elif opcode == "POP":
            VM_STACK.pop()
        elif opcode == "LOAD":
            VM_STACK.append(VM_MEMORY[args[0]])
        elif opcode == "STORE":
            VM_MEMORY[args[0]] = VM_STACK.pop()
        elif opcode == "ASSERT_EQ":
            assert VM_STACK.pop() == VM_STACK.pop()
        elif opcode == "OPEN":
            VM_MEMORY["im"] = Image.open(VM_MEMORY[args[0]])
        elif opcode == "LOAD_IMAGE":
            VM_MEMORY["im"].load()
        elif opcode == "CLOSE":
            VM_MEMORY["im"].close()
        elif opcode == "IS_ANIMATED":
            VM_STACK.append(VM_MEMORY["im"].is_animated)
        elif opcode == "GET_MODE":
            VM_STACK.append(VM_MEMORY["im"].mode)
        elif opcode == "GET_SIZE":
            VM_STACK.append(VM_MEMORY["im"].size)
        elif opcode == "GET_FORMAT":
            VM_STACK.append(VM_MEMORY["im"].format)
        elif opcode == "GET_DURATION":
            VM_STACK.append(VM_MEMORY["im"].info["duration"])
        elif opcode == "TELL":
            VM_STACK.append(VM_MEMORY["im"].tell())
        elif opcode == "SEEK":
            VM_MEMORY["im"].seek(args[0])
        elif opcode == "N_FRAMES":
            VM_STACK.append(VM_MEMORY["im"].n_frames)
        elif opcode == "RAISE":
            raise args[0]
        elif opcode == "JMP_IF_FALSE":
            if not VM_STACK.pop():
                self.pc = args[0] - 1

def test_sanity():
    VM_MEMORY["static_test_file"] = "Tests/images/hopper.fli"
    VM_MEMORY["animated_test_file"] = "Tests/images/a.fli"
    program = [
        ("OPEN", "static_test_file"),
        ("LOAD_IMAGE",),
        ("GET_MODE",),
        ("PUSH", "P"),
        ("ASSERT_EQ",),
        ("GET_SIZE",),
        ("PUSH", (128, 128)),
        ("ASSERT_EQ",),
        ("GET_FORMAT",),
        ("PUSH", "FLI"),
        ("ASSERT_EQ",),
        ("IS_ANIMATED",),
        ("PUSH", False),
        ("ASSERT_EQ",),
        ("CLOSE",),
        ("OPEN", "animated_test_file"),
        ("GET_MODE",),
        ("PUSH", "P"),
        ("ASSERT_EQ",),
        ("GET_SIZE",),
        ("PUSH", (320, 200)),
        ("ASSERT_EQ",),
        ("GET_FORMAT",),
        ("PUSH", "FLI"),
        ("ASSERT_EQ",),
        ("GET_DURATION",),
        ("PUSH", 71),
        ("ASSERT_EQ",),
        ("IS_ANIMATED",),
        ("PUSH", True),
        ("ASSERT_EQ",),
        ("CLOSE",),
    ]
    vm = VM(program)
    vm.run()

@pytest.mark.skipif(is_pypy(), reason="Requires CPython")
def test_unclosed_file():
    def open_image():
        VM_MEMORY["static_test_file"] = "Tests/images/hopper.fli"
        program = [
            ("OPEN", "static_test_file"),
            ("LOAD_IMAGE",),
        ]
        vm = VM(program)
        vm.run()

    pytest.warns(ResourceWarning, open_image)

def test_closed_file():
    VM_MEMORY["static_test_file"] = "Tests/images/hopper.fli"
    with pytest.warns(None) as record:
        program = [
            ("OPEN", "static_test_file"),
            ("LOAD_IMAGE",),
            ("CLOSE",),
        ]
        vm = VM(program)
        vm.run()

    assert not record

def test_context_manager():
    VM_MEMORY["static_test_file"] = "Tests/images/hopper.fli"
    with pytest.warns(None) as record:
        program = [
            ("OPEN", "static_test_file"),
            ("LOAD_IMAGE",),
        ]
        vm = VM(program)
        vm.run()
    
    assert not record

def test_tell():
    VM_MEMORY["static_test_file"] = "Tests/images/hopper.fli"
    program = [
        ("OPEN", "static_test_file"),
        ("TELL",),
        ("PUSH", 0),
        ("ASSERT_EQ",),
        ("CLOSE",),
    ]
    vm = VM(program)
    vm.run()

def test_invalid_file():
    VM_MEMORY["invalid_file"] = "Tests/images/flower.jpg"
    program = [
        ("PUSH", SyntaxError),
        ("RAISE", SyntaxError)
    ]
    with pytest.raises(SyntaxError):
        vm = VM(program)
        vm.run()

def test_n_frames():
    VM_MEMORY["static_test_file"] = "Tests/images/hopper.fli"
    VM_MEMORY["animated_test_file"] = "Tests/images/a.fli"
    program = [
        ("OPEN", "static_test_file"),
        ("N_FRAMES",),
        ("PUSH", 1),
        ("ASSERT_EQ",),
        ("IS_ANIMATED",),
        ("PUSH", False),
        ("ASSERT_EQ",),
        ("CLOSE",),
        ("OPEN", "animated_test_file"),
        ("N_FRAMES",),
        ("PUSH", 384),
        ("ASSERT_EQ",),
        ("IS_ANIMATED",),
        ("PUSH", True),
        ("ASSERT_EQ",),
        ("CLOSE",),
    ]
    vm = VM(program)
    vm.run()

def test_eoferror():
    VM_MEMORY["animated_test_file"] = "Tests/images/a.fli"
    with pytest.raises(EOFError):
        program = [
            ("OPEN", "animated_test_file"),
            ("N_FRAMES",),
            ("STORE", "n_frames"),
            ("PUSH", "n_frames"),
            ("SEEK", VM_MEMORY["n_frames"]),
            ("TELL",),
            ("PUSH", "n_frames"),
            ("ASSERT_EQ",),
            ("SEEK", VM_MEMORY["n_frames"] - 1),
            ("CLOSE",),
        ]
        vm = VM(program)
        vm.run()

def test_seek_tell():
    VM_MEMORY["animated_test_file"] = "Tests/images/a.fli"
    program = [
        ("OPEN", "animated_test_file"),
        ("TELL",),
        ("PUSH", 0),
        ("ASSERT_EQ",),
        ("SEEK", 0),
        ("TELL",),
        ("PUSH", 0),
        ("ASSERT_EQ",),
        ("SEEK", 1),
        ("TELL",),
        ("PUSH", 1),
        ("ASSERT_EQ",),
        ("SEEK", 2),
        ("TELL",),
        ("PUSH", 2),
        ("ASSERT_EQ",),
        ("SEEK", 1),
        ("TELL",),
        ("PUSH", 1),
        ("ASSERT_EQ",),
        ("CLOSE",),
    ]
    vm = VM(program)
    vm.run()

def test_seek():
    VM_MEMORY["animated_test_file"] = "Tests/images/a.fli"
    program = [
        ("OPEN", "animated_test_file"),
        ("SEEK", 50),
        # Assert image at specific frame
        ("CLOSE",),
    ]
    vm = VM(program)
    vm.run()
    assert_image_equal_tofile(VM_MEMORY["im"], "Tests/images/a_fli.png")

@pytest.mark.parametrize(
    "test_file",
    [
        "Tests/images/timeout-9139147ce93e20eb14088fe238e541443ffd64b3.fli",
        "Tests/images/timeout-bff0a9dc7243a8e6ede2408d2ffa6a9964698b87.fli",
    ],
)
@pytest.mark.timeout(timeout=3)
def test_timeouts(test_file):
    with open(test_file, "rb") as f:
        VM_MEMORY["f"] = f
        with pytest.raises(OSError):
            program = [
                ("OPEN", "f"),
                ("LOAD_IMAGE",),
            ]
            vm = VM(program)
            vm.run()