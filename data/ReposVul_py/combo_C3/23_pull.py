import os
import subprocess
import logging
import time
import argparse
import datetime
from traitlets import Integer, default
from traitlets.config import Configurable
from functools import partial

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.registers = {}

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            opcode, *args = instr
            self.dispatch(opcode, *args)
            self.pc += 1

    def dispatch(self, opcode, *args):
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
        elif opcode == 'JMP':
            self.pc = args[0]
        elif opcode == 'JZ':
            if self.stack[-1] == 0:
                self.pc = args[0]
        elif opcode == 'LOAD':
            self.stack.append(self.registers[args[0]])
        elif opcode == 'STORE':
            self.registers[args[0]] = self.stack.pop()

def compile_git_puller(vm, git_url, repo_dir, branch_name):
    vm.load_program([
        ('PUSH', git_url),
        ('STORE', 'git_url'),
        ('PUSH', repo_dir),
        ('STORE', 'repo_dir'),
        ('PUSH', branch_name),
        ('STORE', 'branch_name'),
        ('LOAD', 'git_url'),
        ('LOAD', 'repo_dir'),
        ('LOAD', 'branch_name'),
        ('CALL', 'pull'),
    ])

def execute_cmd(vm, cmd, **kwargs):
    vm.load_program([
        ('PUSH', cmd),
        ('STORE', 'cmd'),
        ('CALL', '_execute_cmd'),
    ])

def _execute_cmd(vm):
    cmd = vm.registers['cmd']
    kwargs = {}
    kwargs['stdout'] = subprocess.PIPE
    kwargs['stderr'] = subprocess.STDOUT
    proc = subprocess.Popen(cmd, **kwargs)

    buf = []

    def flush():
        line = b''.join(buf).decode('utf8', 'replace')
        buf[:] = []
        return line

    c_last = ''
    try:
        for c in iter(partial(proc.stdout.read, 1), b''):
            if c_last == b'\r' and buf and c != b'\n':
                vm.stack.append(flush())
            buf.append(c)
            if c == b'\n':
                vm.stack.append(flush())
            c_last = c
    finally:
        ret = proc.wait()
        if ret != 0:
            raise subprocess.CalledProcessError(ret, cmd)

class GitPuller(Configurable):
    depth = Integer(
        config=True,
        help="Depth (ie, commit count) of clone operations."
    )

    @default('depth')
    def _depth_default(self):
        return int(os.environ.get('NBGITPULLER_DEPTH', 1))

    def __init__(self, git_url, repo_dir, **kwargs):
        assert git_url
        self.git_url = git_url
        self.branch_name = kwargs.pop("branch")
        self.repo_dir = repo_dir
        newargs = {k: v for k, v in kwargs.items() if v is not None}
        super(GitPuller, self).__init__(**newargs)

    def pull(self):
        vm = VM()
        compile_git_puller(vm, self.git_url, self.repo_dir, self.branch_name)
        vm.run()

def main():
    logging.basicConfig(
        format='[%(asctime)s] %(levelname)s -- %(message)s',
        level=logging.DEBUG)

    parser = argparse.ArgumentParser(description='Synchronizes a github repository with a local repository.')
    parser.add_argument('git_url', help='Url of the repo to sync')
    parser.add_argument('branch_name', default=None, help='Branch of repo to sync', nargs='?')
    parser.add_argument('repo_dir', default='.', help='Path to clone repo under', nargs='?')
    args = parser.parse_args()

    for line in GitPuller(
        args.git_url,
        args.repo_dir,
        branch=args.branch_name if args.branch_name else None
    ).pull():
        print(line)

if __name__ == '__main__':
    main()