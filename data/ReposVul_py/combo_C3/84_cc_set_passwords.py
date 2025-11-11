import re

from cloudinit.distros import ug_util
from cloudinit import log as logging
from cloudinit.ssh_util import update_ssh_config
from cloudinit import subp
from cloudinit import util

from string import ascii_letters, digits

LOG = logging.getLogger(__name__)

PW_SET = (''.join([x for x in ascii_letters + digits
                   if x not in 'loLOI01']))

# VM Instructions
PUSH = 'PUSH'
POP = 'POP'
ADD = 'ADD'
SUB = 'SUB'
LOAD = 'LOAD'
STORE = 'STORE'
JMP = 'JMP'
JZ = 'JZ'
CALL = 'CALL'
RET = 'RET'
END = 'END'

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.memory = {}
        self.program = []

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.pc < len(self.program):
            instr = self.program[self.pc]
            op = instr[0]

            if op == PUSH:
                self.stack.append(instr[1])
            elif op == POP:
                self.stack.pop()
            elif op == ADD:
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif op == SUB:
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a - b)
            elif op == LOAD:
                self.stack.append(self.memory.get(instr[1], 0))
            elif op == STORE:
                self.memory[instr[1]] = self.stack.pop()
            elif op == JMP:
                self.pc = instr[1]
                continue
            elif op == JZ:
                if self.stack.pop() == 0:
                    self.pc = instr[1]
                    continue
            elif op == CALL:
                self.memory['return_address'] = self.pc + 1
                self.pc = instr[1]
                continue
            elif op == RET:
                self.pc = self.memory['return_address']
                continue
            elif op == END:
                break

            self.pc += 1

def handle_ssh_pwauth_vm(vm, pw_auth, service_cmd=None, service_name="ssh"):
    vm.load_program([
        (CALL, 0),  # Call handle_ssh_pwauth logic
        (END,)
    ])
    vm.run()

def handle_vm(vm, _name, cfg, cloud, log, args):
    vm.load_program([
        (CALL, 0),  # Call handle logic
        (END,)
    ])
    vm.run()

def rand_user_password_vm(vm, pwlen=20):
    vm.load_program([
        (CALL, 0),  # Call rand_user_password logic
        (END,)
    ])
    vm.run()

def chpasswd_vm(vm, distro, plist_in, hashed=False):
    vm.load_program([
        (CALL, 0),  # Call chpasswd logic
        (END,)
    ])
    vm.run()

vm = VM()

def handle_ssh_pwauth(pw_auth, service_cmd=None, service_name="ssh"):
    handle_ssh_pwauth_vm(vm, pw_auth, service_cmd, service_name)

def handle(_name, cfg, cloud, log, args):
    handle_vm(vm, _name, cfg, cloud, log, args)

def rand_user_password(pwlen=20):
    return rand_user_password_vm(vm, pwlen)

def chpasswd(distro, plist_in, hashed=False):
    chpasswd_vm(vm, distro, plist_in, hashed)