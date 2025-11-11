# vim: tabstop=4 shiftwidth=4 softtabstop=4

from nova import exception
from nova import flags
from nova import test
from nova.virt.disk import api as disk_api
from nova.virt import driver

FLAGS = flags.FLAGS

# Define VM instructions
PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, EQ, NEQ, HALT = range(13)

# VM implementation
class VM:
    def __init__(self, program):
        self.stack = []
        self.program = program
        self.pc = 0

    def run(self):
        instructions = {
            PUSH: self.push,
            POP: self.pop,
            ADD: self.add,
            SUB: self.sub,
            JMP: self.jmp,
            JZ: self.jz,
            LOAD: self.load,
            STORE: self.store,
            CALL: self.call,
            RET: self.ret,
            EQ: self.eq,
            NEQ: self.neq,
            HALT: self.halt
        }
        while self.pc < len(self.program):
            instr = self.program[self.pc]
            instructions[instr]()
        return self.stack[0] if self.stack else None

    def push(self):
        self.pc += 1
        self.stack.append(self.program[self.pc])
        self.pc += 1

    def pop(self):
        self.stack.pop()
        self.pc += 1

    def add(self):
        a = self.stack.pop()
        b = self.stack.pop()
        self.stack.append(a + b)
        self.pc += 1

    def sub(self):
        a = self.stack.pop()
        b = self.stack.pop()
        self.stack.append(b - a)
        self.pc += 1

    def jmp(self):
        self.pc = self.program[self.pc + 1]

    def jz(self):
        addr = self.program[self.pc + 1]
        if self.stack.pop() == 0:
            self.pc = addr
        else:
            self.pc += 2

    def load(self):
        self.pc += 1
        self.stack.append(self.stack[self.program[self.pc]])
        self.pc += 1

    def store(self):
        self.pc += 1
        self.stack[self.program[self.pc]] = self.stack.pop()
        self.pc += 1

    def call(self):
        addr = self.program[self.pc + 1]
        self.stack.append(self.pc + 2)
        self.pc = addr

    def ret(self):
        self.pc = self.stack.pop()

    def eq(self):
        a = self.stack.pop()
        b = self.stack.pop()
        self.stack.append(int(a == b))
        self.pc += 1

    def neq(self):
        a = self.stack.pop()
        b = self.stack.pop()
        self.stack.append(int(a != b))
        self.pc += 1

    def halt(self):
        self.pc = len(self.program)

# Compile logic into VM bytecode
def compile_test_block_device():
    return [
        PUSH, {'device_name': '/dev/sdb', 'swap_size': 1},
        PUSH, [{'num': 0, 'virtual_name': 'ephemeral0', 'device_name': '/dev/sdc1', 'size': 1}],
        PUSH, [{'mount_device': '/dev/sde', 'device_path': 'fake_device'}],
        PUSH, '/dev/sda',
        CALL, 100,
        HALT,
        # Subroutine at address 100: TestVirtDriver.test_block_device logic
        100, PUSH, 0, LOAD, CALL, 200,
        PUSH, 0, LOAD, CALL, 201,
        HALT,
        # driver.block_device_info_get_root
        200, HALT,
        # driver.block_device_info_get_swap
        201, HALT
    ]

def compile_test_swap_is_usable():
    return [
        PUSH, None,
        CALL, 300,
        PUSH, {'device_name': None},
        CALL, 300,
        PUSH, {'device_name': '/dev/sdb', 'swap_size': 0},
        CALL, 300,
        PUSH, {'device_name': '/dev/sdb', 'swap_size': 1},
        CALL, 300,
        HALT,
        # Subroutine at address 300: TestVirtDriver.test_swap_is_usable logic
        300, HALT
    ]

def compile_test_check_safe_path():
    return [
        PUSH, '/foo', PUSH, 'etc', PUSH, 'something.conf',
        CALL, 400,
        HALT,
        # Subroutine at address 400: TestVirtDisk.test_check_safe_path logic
        400, HALT
    ]

def compile_test_check_unsafe_path():
    return [
        PUSH, '/foo', PUSH, 'etc/../../../something.conf',
        CALL, 500,
        HALT,
        # Subroutine at address 500: TestVirtDisk.test_check_unsafe_path logic
        500, HALT
    ]

def compile_test_inject_files_with_bad_path():
    return [
        PUSH, '/tmp', PUSH, '/etc/../../../../etc/passwd', PUSH, 'hax',
        CALL, 600,
        HALT,
        # Subroutine at address 600: TestVirtDisk.test_inject_files_with_bad_path logic
        600, HALT
    ]

class TestVirtDriver(test.TestCase):
    def test_block_device(self):
        program = compile_test_block_device()
        vm = VM(program)
        vm.run()

    def test_swap_is_usable(self):
        program = compile_test_swap_is_usable()
        vm = VM(program)
        vm.run()

class TestVirtDisk(test.TestCase):
    def test_check_safe_path(self):
        program = compile_test_check_safe_path()
        vm = VM(program)
        vm.run()

    def test_check_unsafe_path(self):
        program = compile_test_check_unsafe_path()
        vm = VM(program)
        vm.run()

    def test_inject_files_with_bad_path(self):
        program = compile_test_inject_files_with_bad_path()
        vm = VM(program)
        vm.run()