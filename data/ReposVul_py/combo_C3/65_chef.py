# -*- coding: utf-8 -*-
'''
Execute chef in server or solo mode
'''

import logging
import os
import tempfile
import salt.utils
import salt.utils.decorators as decorators

log = logging.getLogger(__name__)

def __virtual__():
    if not salt.utils.which('chef-client'):
        return False
    return True

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.pc < len(self.program):
            instr = self.program[self.pc]
            getattr(self, instr[0])(*instr[1:])
            self.pc += 1

    def PUSH(self, value):
        self.stack.append(value)

    def POP(self):
        return self.stack.pop()

    def ADD(self):
        b = self.POP()
        a = self.POP()
        self.PUSH(a + b)

    def SUB(self):
        b = self.POP()
        a = self.POP()
        self.PUSH(a - b)

    def JMP(self, address):
        self.pc = address - 1

    def JZ(self, address):
        if self.POP() == 0:
            self.pc = address - 1

    def LOAD(self, index):
        self.PUSH(self.stack[index])

    def STORE(self, index):
        self.stack[index] = self.POP()

def vm_exec_cmd(cmd_type, *args, **kwargs):
    vm = VM()
    program = [
        ("PUSH", ' '.join(args)),
        ("PUSH", ''.join([f' --{k} {v}' for k, v in kwargs.items() if not k.startswith('__')])),
        ("ADD",),
        ("PUSH", 'Chef command: {0}'),
        ("ADD",),
        ("LOAD", 1),
        ("PUSH", cmd_type),
        ("ADD",),
        ("STORE", 0),
        ("LOAD", 0),
        ("PUSH", False),
        ("STORE", 1),
    ]
    vm.load_program(program)
    vm.run()
    cmd_exec = vm.stack[0]
    log.debug(cmd_exec)
    return __salt__['cmd.run_all'](cmd_exec, python_shell=vm.stack[1])

def _default_logfile(exe_name):
    if salt.utils.is_windows():
        tmp_dir = os.path.join(__opts__['cachedir'], 'tmp')
        if not os.path.isdir(tmp_dir):
            os.mkdir(tmp_dir)
        logfile_tmp = tempfile.NamedTemporaryFile(dir=tmp_dir,
                                                  prefix=exe_name,
                                                  suffix='.log',
                                                  delete=False)
        logfile = logfile_tmp.name
        logfile_tmp.close()
    else:
        logfile = salt.utils.path_join(
            '/var/log',
            '{0}.log'.format(exe_name)
        )
    return logfile

@decorators.which('chef-client')
def client(whyrun=False, localmode=False, logfile=_default_logfile('chef-client'), **kwargs):
    args = ['chef-client', '--no-color', '--once', '--logfile "{0}"'.format(logfile), '--format doc']
    if whyrun:
        args.append('--why-run')
    if localmode:
        args.append('--local-mode')
    return vm_exec_cmd('client', *args, **kwargs)

@decorators.which('chef-solo')
def solo(whyrun=False, logfile=_default_logfile('chef-solo'), **kwargs):
    args = ['chef-solo', '--no-color', '--logfile "{0}"'.format(logfile), '--format doc']
    if whyrun:
        args.append('--why-run')
    return vm_exec_cmd('solo', *args, **kwargs)