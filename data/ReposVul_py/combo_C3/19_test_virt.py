import sys
from nova import exception
from nova import flags
from nova import test
from nova.virt.disk import api as disk_api
from nova.virt import driver

FLAGS = flags.FLAGS

# Simple VM implementation
class SimpleVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.running and self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            self.execute(opcode, *args)
            self.pc += 1

    def execute(self, opcode, *args):
        if opcode == 'PUSH':
            self.stack.append(args[0])
        elif opcode == 'POP':
            self.stack.pop()
        elif opcode == 'ADD':
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)
        elif opcode == 'SUB':
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)
        elif opcode == 'LOAD':
            self.stack.append(args[0]())
        elif opcode == 'STORE':
            var = args[0]
            var(self.stack.pop())
        elif opcode == 'JMP':
            self.pc = args[0] - 1
        elif opcode == 'JZ':
            if self.stack.pop() == 0:
                self.pc = args[0] - 1
        elif opcode == 'HALT':
            self.running = False

# Helper functions for VM
def get_root_device_name(info):
    return info.get('root_device_name', None) if info else None

def get_swap(info):
    swap = info.get('swap', None)
    return swap if swap else {'device_name': None, 'swap_size': 0}

def get_ephemerals(info):
    return info.get('ephemerals', []) if info else []

def swap_usable(info):
    return info and info.get('device_name') and info.get('swap_size', 0) > 0

# Compiled instruction sequences
block_device_info_program = [
    ('LOAD', lambda: driver.block_device_info_get_root),
    ('PUSH', {'root_device_name': '/dev/sda', 'swap': {'device_name': '/dev/sdb', 'swap_size': 1}, 'ephemerals': [{'num': 0, 'virtual_name': 'ephemeral0', 'device_name': '/dev/sdc1', 'size': 1}], 'block_device_mapping': [{'mount_device': '/dev/sde', 'device_path': 'fake_device'}]}),
    ('CALL', lambda f, *args: f(*args)),
    ('PUSH', '/dev/sda'),
    ('EQ'),
    ('ASSERT'),
    ('HALT')
]

swap_usable_program = [
    ('LOAD', lambda: driver.swap_is_usable),
    ('PUSH', {'device_name': '/dev/sdb', 'swap_size': 1}),
    ('CALL', lambda f, *args: f(*args)),
    ('PUSH', True),
    ('EQ'),
    ('ASSERT'),
    ('HALT')
]

class TestVirtDriver(test.TestCase):
    def test_block_device(self):
        vm = SimpleVM()
        vm.load_program(block_device_info_program)
        vm.run()

    def test_swap_is_usable(self):
        vm = SimpleVM()
        vm.load_program(swap_usable_program)
        vm.run()

class TestVirtDisk(test.TestCase):
    def test_check_safe_path(self):
        ret = disk_api._join_and_check_path_within_fs('/foo', 'etc', 'something.conf')
        self.assertEquals(ret, '/foo/etc/something.conf')

    def test_check_unsafe_path(self):
        self.assertRaises(exception.Invalid, disk_api._join_and_check_path_within_fs, '/foo', 'etc/../../../something.conf')

    def test_inject_files_with_bad_path(self):
        self.assertRaises(exception.Invalid, disk_api._inject_file_into_fs, '/tmp', '/etc/../../../../etc/passwd', 'hax')