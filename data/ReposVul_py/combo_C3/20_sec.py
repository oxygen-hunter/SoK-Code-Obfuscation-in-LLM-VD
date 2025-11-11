import sys
import re
import os
from lshell import utils

class VM:
    def __init__(self, memory=None, stack=None):
        self.memory = memory if memory is not None else {}
        self.stack = stack if stack is not None else []
        self.pc = 0
        self.running = True

    def run(self, bytecode):
        while self.running and self.pc < len(bytecode):
            op = bytecode[self.pc]
            self.execute(op)

    def execute(self, op):
        ops = {
            'PUSH': self.op_push,
            'POP': self.op_pop,
            'ADD': self.op_add,
            'SUB': self.op_sub,
            'JMP': self.op_jmp,
            'JZ': self.op_jz,
            'LOAD': self.op_load,
            'STORE': self.op_store,
            'HALT': self.op_halt,
        }
        if op[0] in ops:
            ops[op[0]](op)
        else:
            raise Exception(f'Unknown operation {op[0]}')

    def op_push(self, op):
        self.stack.append(op[1])
        self.pc += 1

    def op_pop(self, op):
        self.stack.pop()
        self.pc += 1

    def op_add(self, op):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)
        self.pc += 1

    def op_sub(self, op):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)
        self.pc += 1

    def op_jmp(self, op):
        self.pc = op[1]

    def op_jz(self, op):
        if self.stack[-1] == 0:
            self.pc = op[1]
        else:
            self.pc += 1

    def op_load(self, op):
        self.stack.append(self.memory[op[1]])
        self.pc += 1

    def op_store(self, op):
        self.memory[op[1]] = self.stack.pop()
        self.pc += 1

    def op_halt(self, op):
        self.running = False


def run_vm_program():
    vm = VM()
    bytecode = [
        ('PUSH', 10),        # simulate initialization
        ('STORE', 'warnings'),
        ('PUSH', 0),
        ('STORE', 'ret_code'),
        ('PUSH', 'path'),    # simulate path check
        ('LOAD', 'path'),
        ('JZ', 11),          # if path invalid, jump to warning
        ('PUSH', 0),
        ('STORE', 'ret_code'),
        ('JMP', 12),
        ('PUSH', 1),         # simulate warning
        ('STORE', 'ret_code'),
        ('HALT',)
    ]
    vm.run(bytecode)
    return vm.memory['ret_code'], {'warning_counter': vm.memory['warnings'], 'logpath': 'log', 'quiet': False}


def warn_count(messagetype, command, conf, strict=None, ssh=None):
    ret, conf = run_vm_program()
    return ret, conf


def check_path(line, conf, completion=None, ssh=None, strict=None):
    allowed_path_re = str(conf['path'][0])
    denied_path_re = str(conf['path'][1][:-1])
    sep = re.compile(r'\ |;|\||&')
    line = line.strip()
    line = sep.split(line)

    for item in line:
        item = re.sub(r'^["\'`]|["\'`]$', '', item)
        item = re.sub(r'^\$[\(\{]|[\)\}]$', '', item)
        if type(item) not in ['str', 'int']:
            item = str(item)
        item = os.path.expanduser(item)

        if re.findall('\$|\*|\?', item):
            item = re.sub("\"|\'", "", item)
            import subprocess
            p = subprocess.Popen("`which echo` %s" % item,
                                 shell=True,
                                 stdin=subprocess.PIPE,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
            cout = p.stdout

            try:
                item = cout.readlines()[0].decode('utf8').split(' ')[0]
                item = item.strip()
                item = os.path.expandvars(item)
            except IndexError:
                conf['logpath'].critical('*** Internal error: command not executed')
                return 1, conf

        tomatch = os.path.realpath(item)
        if os.path.isdir(tomatch) and tomatch[-1] != '/':
            tomatch += '/'
        match_allowed = re.findall(allowed_path_re, tomatch)
        match_denied = re.findall(denied_path_re, tomatch) if denied_path_re else None

        if not match_allowed or match_denied:
            if not completion:
                ret, conf = warn_count('path', tomatch, conf, strict=strict, ssh=ssh)
            return 1, conf

    if not completion:
        if not re.findall(allowed_path_re, os.getcwd() + '/'):
            ret, conf = warn_count('path', tomatch, conf, strict=strict, ssh=ssh)
            os.chdir(conf['home_path'])
            conf['promptprint'] = utils.updateprompt(os.getcwd(), conf)
            return 1, conf
    return 0, conf


def check_secure(line, conf, strict=None, ssh=None):
    oline = line
    line = line.strip()
    returncode = 0

    relist = re.findall(r'[^=]\"(.+)\"', line) + re.findall(r'[^=]\'(.+)\'', line)
    for item in relist:
        if os.path.exists(item):
            ret_check_path, conf = check_path(item, conf, strict=strict)
            returncode += ret_check_path

    if re.findall(r'[\x01-\x1F\x7F]', oline):
        ret, conf = warn_count('control char', oline, conf, strict=strict, ssh=ssh)
        return ret, conf

    for item in conf['forbidden']:
        if item in ['&', '|']:
            if re.findall("[^\%s]\%s[^\%s]" % (item, item, item), line):
                ret, conf = warn_count('syntax', oline, conf, strict=strict, ssh=ssh)
                return ret, conf
        else:
            if item in line:
                ret, conf = warn_count('syntax', oline, conf, strict=strict, ssh=ssh)
                return ret, conf

    executions = re.findall('\$\([^)]+[)]', line)
    for item in executions:
        ret_check_path, conf = check_path(item[2:-1].strip(), conf, strict=strict)
        returncode += ret_check_path
        ret_check_secure, conf = check_secure(item[2:-1].strip(), conf, strict=strict)
        returncode += ret_check_secure

    executions = re.findall('\`[^`]+[`]', line)
    for item in executions:
        ret_check_secure, conf = check_secure(item[1:-1].strip(), conf, strict=strict)
        returncode += ret_check_secure

    curly = re.findall('\$\{[^}]+[}]', line)
    for item in curly:
        if re.findall(r'=|\+|\?|\-', item):
            variable = re.split('=|\+|\?|\-', item, 1)
        else:
            variable = item
        ret_check_path, conf = check_path(variable[1][:-1], conf, strict=strict)
        returncode += ret_check_path

    if returncode > 0:
        return 1, conf
    elif line.startswith('$(') or line.startswith('`'):
        return 0, conf

    lines = []
    if line[0] in ["&", "|", ";"]:
        start = 1
    else:
        start = 0

    for i in range(1, len(line)):
        if line[i] in ["&", "|", ";"] and line[i - 1] != "\\":
            if start != i:
                lines.append(line[start:i])
            start = i + 1

    if start != len(line):
        lines.append(line[start:len(line)])

    line = re.sub('\)$', '', line)
    for separate_line in lines:
        separate_line = " ".join(separate_line.split())
        splitcmd = separate_line.strip().split(' ')
        command = splitcmd[0]
        cmdargs = splitcmd if len(splitcmd) > 1 else None

        if command == 'sudo':
            if type(cmdargs) == list:
                if cmdargs[1] == '-u' and cmdargs:
                    sudocmd = cmdargs[3]
                else:
                    sudocmd = cmdargs[1]
                if sudocmd not in conf['sudo_commands'] and cmdargs:
                    ret, conf = warn_count('sudo command', oline, conf, strict=strict, ssh=ssh)
                    return ret, conf

        if ssh:
            conf['allowed'] = conf['overssh']

        if command not in conf['allowed'] and command:
            ret, conf = warn_count('command', command, conf, strict=strict, ssh=ssh)
            return ret, conf
    return 0, conf