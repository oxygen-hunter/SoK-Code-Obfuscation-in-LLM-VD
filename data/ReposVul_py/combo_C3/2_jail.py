from __future__ import (absolute_import, division, print_function)
__metaclass__ = type

import distutils.spawn
import traceback
import os
import subprocess
from ansible import errors
from ansible.callbacks import vvv
import ansible.constants as C

BUFSIZE = 4096

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = {
            'PUSH': self.push,
            'POP': self.pop,
            'ADD': self.add,
            'SUB': self.sub,
            'JMP': self.jmp,
            'JZ': self.jz,
            'LOAD': self.load,
            'STORE': self.store,
        }
        self.memory = {}

    def run(self, bytecode):
        self.bytecode = bytecode
        while self.pc < len(self.bytecode):
            instr, *args = self.bytecode[self.pc]
            self.instructions[instr](*args)
            self.pc += 1

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        return self.stack.pop()

    def add(self):
        b = self.pop()
        a = self.pop()
        self.push(a + b)

    def sub(self):
        b = self.pop()
        a = self.pop()
        self.push(a - b)

    def jmp(self, addr):
        self.pc = addr - 1

    def jz(self, addr):
        if self.pop() == 0:
            self.pc = addr - 1

    def load(self, addr):
        self.push(self.memory.get(addr, 0))

    def store(self, addr):
        self.memory[addr] = self.pop()

class Connection(object):
    def __init__(self, runner, host, port, *args, **kwargs):
        self.vm = VirtualMachine()
        self.jail = host
        self.runner = runner
        self.host = host
        self.has_pipelining = False
        self.become_methods_supported = C.BECOME_METHODS

        if os.geteuid() != 0:
            raise errors.AnsibleError("jail connection requires running as root")

        self.jls_cmd = self._search_executable('jls')
        self.jexec_cmd = self._search_executable('jexec')
        
        bytecode = [
            ('PUSH', self.jail),
            ('PUSH', self.jls_cmd),
            ('LOAD', 'jail_list'),
            ('STORE', 'jail_list'),
            ('LOAD', 'jail_list'),
            ('JZ', 9),
            ('POP', ),
            ('JMP', 10),
            ('POP', ),
        ]
        self.vm.run(bytecode)

        if self.jail not in self.vm.memory['jail_list']:
            raise errors.AnsibleError("incorrect jail name %s" % self.jail)

        self.host = host
        self.port = port

    def _search_executable(self, executable):
        cmd = distutils.spawn.find_executable(executable)
        if not cmd:
            raise errors.AnsibleError("%s command not found in PATH") % executable
        return cmd

    def list_jails(self):
        p = subprocess.Popen([self.jls_cmd, '-q', 'name'],
                             cwd=self.runner.basedir,
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        stdout, stderr = p.communicate()
        return stdout.split()

    def get_jail_path(self):
        p = subprocess.Popen([self.jls_cmd, '-j', self.jail, '-q', 'path'],
                             cwd=self.runner.basedir,
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        stdout, stderr = p.communicate()
        return stdout[:-1]

    def connect(self, port=None):
        vvv("THIS IS A LOCAL JAIL DIR", host=self.jail)
        return self

    def _generate_cmd(self, executable, cmd):
        if executable:
            local_cmd = [self.jexec_cmd, self.jail, executable, '-c', cmd]
        else:
            local_cmd = '%s "%s" %s' % (self.jexec_cmd, self.jail, cmd)
        return local_cmd

    def _buffered_exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None, stdin=subprocess.PIPE):
        if sudoable and self.runner.become and self.runner.become_method not in self.become_methods_supported:
            raise errors.AnsibleError("Internal Error: this module does not support running commands via %s" % self.runner.become_method)

        if in_data:
            raise errors.AnsibleError("Internal Error: this module does not support optimized module pipelining")

        local_cmd = self._generate_cmd(executable, cmd)
        vvv("EXEC %s" % (local_cmd), host=self.jail)
        p = subprocess.Popen(local_cmd, shell=isinstance(local_cmd, str),
                             cwd=self.runner.basedir,
                             stdin=stdin,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return p

    def exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None):
        p = self._buffered_exec_command(cmd, tmp_path, become_user, sudoable, executable, in_data)
        stdout, stderr = p.communicate()
        return (p.returncode, '', stdout, stderr)

    def put_file(self, in_path, out_path):
        vvv("PUT %s TO %s" % (in_path, out_path), host=self.jail)
        with open(in_path, 'rb') as in_file:
            p = self._buffered_exec_command('dd of=%s' % out_path, None, stdin=in_file)
            try:
                stdout, stderr = p.communicate()
            except:
                traceback.print_exc()
                raise errors.AnsibleError("failed to transfer file to %s" % out_path)
            if p.returncode != 0:
                raise errors.AnsibleError("failed to transfer file to %s:\n%s\n%s" % (out_path, stdout, stderr))

    def fetch_file(self, in_path, out_path):
        vvv("FETCH %s TO %s" % (in_path, out_path), host=self.jail)
        p = self._buffered_exec_command('dd if=%s bs=%s' % (in_path, BUFSIZE), None)
        with open(out_path, 'wb+') as out_file:
            try:
                for chunk in p.stdout.read(BUFSIZE):
                    out_file.write(chunk)
            except:
                traceback.print_exc()
                raise errors.AnsibleError("failed to transfer file to %s" % out_path)
            stdout, stderr = p.communicate()
            if p.returncode != 0:
                raise errors.AnsibleError("failed to transfer file to %s:\n%s\n%s" % (out_path, stdout, stderr))

    def close(self):
        pass