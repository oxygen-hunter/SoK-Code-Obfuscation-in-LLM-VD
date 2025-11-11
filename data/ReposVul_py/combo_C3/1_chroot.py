from __future__ import (absolute_import, division, print_function)
__metaclass__ = type

import distutils.spawn
import traceback
import os
import subprocess
from ansible import errors
from ansible import utils
from ansible.callbacks import vvv
import ansible.constants as C

BUFSIZE = 65536

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.running and self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.pc += 1
            self.execute(instr)

    def execute(self, instr):
        opcode = instr[0]
        if opcode == 'PUSH':
            self.stack.append(instr[1])
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
        elif opcode == 'JMP':
            self.pc = instr[1]
        elif opcode == 'JZ':
            if self.stack.pop() == 0:
                self.pc = instr[1]
        elif opcode == 'LOAD':
            self.stack.append(instr[1])
        elif opcode == 'STORE':
            self.stack[instr[1]] = self.stack.pop()
        elif opcode == 'HALT':
            self.running = False
        else:
            raise RuntimeError("Unknown instruction: %s" % opcode)

class Connection(object):
    def __init__(self, runner, host, port, *args, **kwargs):
        vm = VM()
        vm.load_instructions([
            ('LOAD', os.geteuid()),
            ('PUSH', 0),
            ('SUB',),
            ('JZ', 7),
            ('HALT',),
            ('LOAD', host),
            ('LOAD', self.is_dir(host)),
            ('JZ', 7),
            ('HALT',),
            ('LOAD', os.path.join(host, 'bin/sh')),
            ('LOAD', self.is_executable(os.path.join(host, 'bin/sh'))),
            ('JZ', 7),
            ('HALT',),
            ('LOAD', distutils.spawn.find_executable('chroot')),
            ('STORE', 0),
            ('LOAD', runner),
            ('STORE', 1),
            ('LOAD', host),
            ('STORE', 2),
            ('LOAD', port),
            ('STORE', 3),
            ('HALT',)
        ])
        vm.run()

        self.chroot = vm.stack[2]
        self.runner = vm.stack[1]
        self.host = vm.stack[2]
        self.port = vm.stack[3]
        self.chroot_cmd = vm.stack[0]
        self.has_pipelining = False
        self.become_methods_supported = C.BECOME_METHODS

    def is_dir(self, path):
        return os.path.isdir(path)

    def is_executable(self, path):
        return utils.is_executable(path)

    def connect(self, port=None):
        vm = VM()
        vm.load_instructions([
            ('LOAD', self.chroot),
            ('HALT',)
        ])
        vm.run()
        vvv("THIS IS A LOCAL CHROOT DIR", host=vm.stack[0])
        return self

    def _generate_cmd(self, executable, cmd):
        vm = VM()
        vm.load_instructions([
            ('LOAD', executable),
            ('PUSH', None),
            ('JZ', 5),
            ('LOAD', [self.chroot_cmd, self.chroot, executable, '-c', cmd]),
            ('STORE', 0),
            ('JMP', 7),
            ('LOAD', '%s "%s" %s' % (self.chroot_cmd, self.chroot, cmd)),
            ('STORE', 0),
            ('HALT',)
        ])
        vm.run()
        return vm.stack[0]

    def _buffered_exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None, stdin=subprocess.PIPE):
        vm = VM()
        vm.load_instructions([
            ('LOAD', sudoable),
            ('LOAD', self.runner.become),
            ('LOAD', self.runner.become_method),
            ('LOAD', self.become_methods_supported),
            ('JZ', 11),
            ('LOAD', in_data),
            ('JZ', 11),
            ('HALT',),
            ('LOAD', executable),
            ('LOAD', cmd),
            ('LOAD', self._generate_cmd(executable, cmd)),
            ('STORE', 0),
            ('HALT',)
        ])
        vm.run()
        local_cmd = vm.stack[0]

        vvv("EXEC %s" % (local_cmd), host=self.chroot)
        p = subprocess.Popen(local_cmd, shell=isinstance(local_cmd, basestring),
                             cwd=self.runner.basedir,
                             stdin=stdin,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return p

    def exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None):
        p = self._buffered_exec_command(cmd, tmp_path, become_user, sudoable, executable, in_data)
        stdout, stderr = p.communicate()
        return (p.returncode, '', stdout, stderr)

    def put_file(self, in_path, out_path):
        vm = VM()
        vm.load_instructions([
            ('LOAD', in_path),
            ('LOAD', out_path),
            ('HALT',)
        ])
        vm.run()
        vvv("PUT %s TO %s" % (vm.stack[0], vm.stack[1]), host=self.chroot)

        try:
            with open(in_path, 'rb') as in_file:
                try:
                    p = self._buffered_exec_command('dd of=%s' % out_path, None, stdin=in_file)
                except OSError:
                    raise errors.AnsibleError("chroot connection requires dd command in the chroot")
                try:
                    stdout, stderr = p.communicate()
                except:
                    traceback.print_exc()
                    raise errors.AnsibleError("failed to transfer file %s to %s" % (in_path, out_path))
                if p.returncode != 0:
                    raise errors.AnsibleError("failed to transfer file %s to %s:\n%s\n%s" % (in_path, out_path, stdout, stderr))
        except IOError:
            raise errors.AnsibleError("file or module does not exist at: %s" % in_path)

    def fetch_file(self, in_path, out_path):
        vm = VM()
        vm.load_instructions([
            ('LOAD', in_path),
            ('LOAD', out_path),
            ('HALT',)
        ])
        vm.run()
        vvv("FETCH %s TO %s" % (vm.stack[0], vm.stack[1]), host=self.chroot)

        try:
            p = self._buffered_exec_command('dd if=%s bs=%s' % (in_path, BUFSIZE), None)
        except OSError:
            raise errors.AnsibleError("chroot connection requires dd command in the jail")

        with open(out_path, 'wb+') as out_file:
            try:
                for chunk in p.stdout.read(BUFSIZE):
                    out_file.write(chunk)
            except:
                traceback.print_exc()
                raise errors.AnsibleError("failed to transfer file %s to %s" % (in_path, out_path))
            stdout, stderr = p.communicate()
            if p.returncode != 0:
                raise errors.AnsibleError("failed to transfer file %s to %s:\n%s\n%s" % (in_path, out_path, stdout, stderr))

    def close(self):
        pass