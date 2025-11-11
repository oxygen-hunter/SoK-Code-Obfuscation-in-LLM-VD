import pytest

from vyper import compiler
from vyper.exceptions import InvalidType, StructureException

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True

    def load_program(self, program):
        self.instructions = program

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        return self.stack.pop() if self.stack else None

    def execute(self):
        while self.running and self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.pc += 1
            instr(self)

def PUSH(value):
    return lambda vm: vm.push(value)

def POP():
    return lambda vm: vm.pop()

def RAISE(exception):
    return lambda vm: exec(f"raise {exception}")

def COMPILE(bad_code):
    return lambda vm: compiler.compile_code(bad_code)

program = [
    PUSH(0), RAISE("StructureException"),
    PUSH(1), RAISE("InvalidType"),
    PUSH(2), COMPILE("x[5] = 4"),
    PUSH(3), COMPILE("send(0x1234567890123456789012345678901234567890, 5)"),
    PUSH(4), COMPILE("send(0x1234567890123456789012345678901234567890, 5)"),
    PUSH(5), COMPILE("x: [bar, baz]"),
    PUSH(6), COMPILE("x: [bar(int128), baz(baffle)]"),
    PUSH(7), COMPILE("x: int128\n@external\n@view(123)\ndef foo() -> int128:\n    pass"),
    PUSH(8), COMPILE("@external\ndef foo():\n    throe"),
    PUSH(9), COMPILE("@external\ndef foo() -> int128:\n    x: address = 0x1234567890123456789012345678901234567890\n    return x.balance()"),
    PUSH(10), COMPILE("@external\ndef foo() -> int128:\n    x: address = 0x1234567890123456789012345678901234567890\n    return x.codesize()"),
    PUSH(11), COMPILE("@external\n@nonreentrant(\"B\")\n@nonreentrant(\"C\")\ndef double_nonreentrant():\n    pass"),
    PUSH(12), COMPILE("struct X:\n    int128[5]: int128[7]"),
    PUSH(13), COMPILE("@external\n@nonreentrant(\" \")\ndef invalid_nonreentrant_key():\n    pass"),
    PUSH(14), COMPILE("@external\n@nonreentrant(\"\")\ndef invalid_nonreentrant_key():\n    pass"),
    PUSH(15), COMPILE("@external\n@nonreentrant(\"123\")\ndef invalid_nonreentrant_key():\n    pass"),
    PUSH(16), COMPILE("@external\n@nonreentrant(\"!123abcd\")\ndef invalid_nonreentrant_key():\n    pass"),
    PUSH(17), COMPILE("@external\ndef foo():\n    true: int128 = 3"),
    PUSH(18), COMPILE("n: HashMap[uint256, bool][3]"),
    PUSH(19), COMPILE("a: constant(uint256) = 3\nn: public(HashMap[uint256, uint256][a])"),
    PUSH(20), COMPILE("a: immutable(uint256)\nn: public(HashMap[uint256, bool][a])\n\n@external\ndef __init__():\n    a = 3"),
    PUSH(21), COMPILE("n: HashMap[uint256, bool][3][3]"),
    PUSH(22), COMPILE("m1: HashMap[uint8, uint8]\nm2: HashMap[uint8, uint8]\n\n@external\ndef __init__():\n    self.m1 = self.m2"),
    PUSH(23), COMPILE("m1: HashMap[uint8, uint8]\n\n@external\ndef __init__():\n    self.m1 = 234"),
]

@pytest.mark.parametrize("index", range(len(program) // 2))
def test_invalid_type_exception(index):
    vm = VM()
    vm.load_program(program[index * 2:index * 2 + 2])
    with pytest.raises((StructureException, InvalidType)):
        vm.execute()

del_fail_list = [
    """
x: int128(address)
    """,
    """
x: int128(2 ** 2)
    """,
    """
# invalid interface declaration (pass)
interface Bar:
    def set_lucky(arg1: int128): pass
    """,
    """
interface Bar:
# invalud interface declaration (assignment)
    def set_lucky(arg1: int128):
        arg1 = 1
        arg1 = 3
    """,
]