import logging
from paramiko.py3compat import byte_chr, PY2, bytes_types, text_type, long

class VM:
    def __init__(self):
        self.stack = []
        self.registers = {}
        self.pc = 0
        self.instructions = []
        self.running = True

    def load_program(self, program):
        self.instructions = program

    def run(self):
        while self.running and self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.execute(instr)
            self.pc += 1

    def execute(self, instr):
        op = instr[0]
        if op == 'PUSH':
            self.stack.append(instr[1])
        elif op == 'POP':
            self.stack.pop()
        elif op == 'ADD':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a + b)
        elif op == 'SUB':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a - b)
        elif op == 'LOAD':
            self.stack.append(self.registers[instr[1]])
        elif op == 'STORE':
            self.registers[instr[1]] = self.stack.pop()
        elif op == 'JMP':
            self.pc = instr[1] - 1
        elif op == 'JZ':
            if self.stack.pop() == 0:
                self.pc = instr[1] - 1
        elif op == 'HALT':
            self.running = False

def create_message_constants(vm):
    prog = [
        ('PUSH', 1), ('STORE', 'MSG_DISCONNECT'),
        ('PUSH', 2), ('STORE', 'MSG_IGNORE'),
        ('PUSH', 3), ('STORE', 'MSG_UNIMPLEMENTED'),
        ('PUSH', 4), ('STORE', 'MSG_DEBUG'),
        ('PUSH', 5), ('STORE', 'MSG_SERVICE_REQUEST'),
        ('PUSH', 6), ('STORE', 'MSG_SERVICE_ACCEPT'),
        ('PUSH', 20), ('STORE', 'MSG_KEXINIT'),
        ('PUSH', 21), ('STORE', 'MSG_NEWKEYS'),
        # Additional constants here...
        ('HALT',)
    ]
    vm.load_program(prog)
    vm.run()

def create_byte_constants(vm):
    prog = [
        ('LOAD', 'MSG_DISCONNECT'), ('PUSH', 0), ('ADD'), ('STORE', 'cMSG_DISCONNECT'),
        ('LOAD', 'MSG_IGNORE'), ('PUSH', 0), ('ADD'), ('STORE', 'cMSG_IGNORE'),
        ('LOAD', 'MSG_UNIMPLEMENTED'), ('PUSH', 0), ('ADD'), ('STORE', 'cMSG_UNIMPLEMENTED'),
        ('LOAD', 'MSG_DEBUG'), ('PUSH', 0), ('ADD'), ('STORE', 'cMSG_DEBUG'),
        # Additional byte constants here...
        ('HALT',)
    ]
    vm.load_program(prog)
    vm.run()

vm = VM()
create_message_constants(vm)
create_byte_constants(vm)

MSG_NAMES = {
    vm.registers['MSG_DISCONNECT']: 'disconnect',
    vm.registers['MSG_IGNORE']: 'ignore',
    vm.registers['MSG_UNIMPLEMENTED']: 'unimplemented',
    vm.registers['MSG_DEBUG']: 'debug',
    vm.registers['MSG_SERVICE_REQUEST']: 'service-request',
    vm.registers['MSG_SERVICE_ACCEPT']: 'service-accept',
    vm.registers['MSG_KEXINIT']: 'kexinit',
    vm.registers['MSG_NEWKEYS']: 'newkeys',
    # Additional message names here...
}

AUTH_SUCCESSFUL, AUTH_PARTIALLY_SUCCESSFUL, AUTH_FAILED = range(3)
(OPEN_SUCCEEDED, OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED,
 OPEN_FAILED_CONNECT_FAILED, OPEN_FAILED_UNKNOWN_CHANNEL_TYPE,
 OPEN_FAILED_RESOURCE_SHORTAGE) = range(0, 5)

CONNECTION_FAILED_CODE = {
    1: 'Administratively prohibited',
    2: 'Connect failed',
    3: 'Unknown channel type',
    4: 'Resource shortage'
}

DISCONNECT_SERVICE_NOT_AVAILABLE, DISCONNECT_AUTH_CANCELLED_BY_USER, \
    DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE = 7, 13, 14

zero_byte = byte_chr(0)
one_byte = byte_chr(1)
four_byte = byte_chr(4)
max_byte = byte_chr(0xff)
cr_byte = byte_chr(13)
linefeed_byte = byte_chr(10)
crlf = cr_byte + linefeed_byte

if PY2:
    cr_byte_value = cr_byte
    linefeed_byte_value = linefeed_byte
else:
    cr_byte_value = 13
    linefeed_byte_value = 10

def asbytes(s):
    if isinstance(s, bytes_types):
        return s
    if isinstance(s, text_type):
        return s.encode("utf-8")
    asbytes = getattr(s, "asbytes", None)
    if asbytes is not None:
        return asbytes()
    return s

xffffffff = long(0xffffffff)
x80000000 = long(0x80000000)
o666 = 438
o660 = 432
o644 = 420
o600 = 384
o777 = 511
o700 = 448
o70 = 56

DEBUG = logging.DEBUG
INFO = logging.INFO
WARNING = logging.WARNING
ERROR = logging.ERROR
CRITICAL = logging.CRITICAL

io_sleep = 0.01
DEFAULT_WINDOW_SIZE = 64 * 2 ** 15
DEFAULT_MAX_PACKET_SIZE = 2 ** 15
MIN_WINDOW_SIZE = 2 ** 15
MIN_PACKET_SIZE = 2 ** 12
MAX_WINDOW_SIZE = 2 ** 32 - 1