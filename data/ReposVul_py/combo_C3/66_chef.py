# -*- coding: utf-8 -*-
import logging
import os
import tempfile
import salt.utils
import salt.utils.decorators as decorators

log = logging.getLogger(__name__)

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.memory = {}
        self.halted = False

    def load_instructions(self, instructions):
        self.instructions = instructions

    def fetch(self):
        if self.pc < len(self.instructions):
            return self.instructions[self.pc]
        return None

    def execute(self):
        while not self.halted:
            instr = self.fetch()
            if instr is None:
                break
            self.pc += 1
            self.run_instruction(instr)

    def run_instruction(self, instr):
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
        elif op == 'JMP':
            self.pc = instr[1]
        elif op == 'JZ':
            if self.stack.pop() == 0:
                self.pc = instr[1]
        elif op == 'LOAD':
            self.stack.append(self.memory[instr[1]])
        elif op == 'STORE':
            self.memory[instr[1]] = self.stack.pop()
        elif op == 'CALL':
            func = instr[1]
            args = [self.stack.pop() for _ in range(instr[2])]
            func(*args)
        elif op == 'HALT':
            self.halted = True

def __virtual__():
    vm = VM()
    vm.load_instructions([
        ('CALL', lambda: salt.utils.which('chef-client') is not None, 0),
        ('STORE', 'result'),
        ('LOAD', 'result'),
        ('JZ', 6),
        ('PUSH', True),
        ('HALT',),
        ('PUSH', False),
        ('HALT',)
    ])
    vm.execute()
    return vm.stack.pop()

def _default_logfile(exe_name):
    vm = VM()
    vm.load_instructions([
        ('CALL', lambda: salt.utils.is_windows(), 0),
        ('STORE', 'is_windows'),
        ('LOAD', 'is_windows'),
        ('JZ', 6),
        ('CALL', lambda: tempfile.NamedTemporaryFile(dir=os.environ['TMP'], prefix=exe_name, suffix='.log', delete=False), 0),
        ('STORE', 'logfile_tmp'),
        ('CALL', lambda: vm.memory['logfile_tmp'].name, 0),
        ('STORE', 'logfile'),
        ('CALL', lambda: vm.memory['logfile_tmp'].close(), 0),
        ('JMP', 12),
        ('CALL', lambda: salt.utils.path_join('/var/log', '{0}.log'.format(exe_name)), 0),
        ('STORE', 'logfile'),
        ('LOAD', 'logfile'),
        ('HALT',)
    ])
    vm.execute()
    return vm.stack.pop()

@decorators.which('chef-client')
def client(whyrun=False, localmode=False, logfile=_default_logfile('chef-client'), **kwargs):
    vm = VM()
    vm.load_instructions([
        ('PUSH', 'chef-client --no-color --once --logfile "{0}" --format doc'.format(logfile)),
        ('STORE', 'args'),
        ('PUSH', whyrun),
        ('JZ', 7),
        ('LOAD', 'args'),
        ('PUSH', ' --why-run'),
        ('ADD',),
        ('STORE', 'args'),
        ('LOAD', localmode),
        ('JZ', 13),
        ('LOAD', 'args'),
        ('PUSH', ' --local-mode'),
        ('ADD',),
        ('STORE', 'args'),
        ('LOAD', 'args'),
        ('PUSH', kwargs),
        ('CALL', _exec_cmd, 2),
        ('HALT',)
    ])
    vm.execute()
    return vm.stack.pop()

@decorators.which('chef-solo')
def solo(whyrun=False, logfile=_default_logfile('chef-solo'), **kwargs):
    vm = VM()
    vm.load_instructions([
        ('PUSH', 'chef-solo --no-color --logfile "{0}" --format doc'.format(logfile)),
        ('STORE', 'args'),
        ('PUSH', whyrun),
        ('JZ', 7),
        ('LOAD', 'args'),
        ('PUSH', ' --why-run'),
        ('ADD',),
        ('STORE', 'args'),
        ('LOAD', 'args'),
        ('PUSH', kwargs),
        ('CALL', _exec_cmd, 2),
        ('HALT',)
    ])
    vm.execute()
    return vm.stack.pop()

def _exec_cmd(*args, **kwargs):
    vm = VM()
    vm.load_instructions([
        ('PUSH', ' '.join(args)),
        ('STORE', 'cmd_args'),
        ('PUSH', ''.join([' --{0} {1}'.format(k, v) for k, v in kwargs.items() if not k.startswith('__')])),
        ('STORE', 'cmd_kwargs'),
        ('LOAD', 'cmd_args'),
        ('LOAD', 'cmd_kwargs'),
        ('ADD',),
        ('STORE', 'cmd_exec'),
        ('CALL', lambda: log.debug('Chef command: {0}'.format(vm.memory['cmd_exec'])), 0),
        ('LOAD', '__salt__'),
        ('CALL', lambda salt: salt['cmd.run_all'](vm.memory['cmd_exec'], python_shell=False), 1),
        ('HALT',)
    ])
    vm.execute()
    return vm.stack.pop()