import sys
import re
import os
import subprocess
from lshell import utils

class SimpleVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.code = []

    def run(self, code):
        self.code = code
        while self.pc < len(self.code):
            instruction = self.code[self.pc]
            getattr(self, f"op_{instruction[0]}")(*instruction[1:])
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a - b)

    def op_JMP(self, address):
        self.pc = address - 1

    def op_JZ(self, address):
        if self.op_POP() == 0:
            self.pc = address - 1

    def op_LOAD(self, index):
        self.op_PUSH(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.op_POP()


def execute_vm_program():
    vm = SimpleVM()
    vm.run([
        ('PUSH', 0),
        ('STORE', 0),  # warning_counter
        ('PUSH', -1),
        ('STORE', 1),  # strict
        ('PUSH', 0),
        ('STORE', 2),  # ssh
        # Simulating logic of warn_count
        ('LOAD', 1),
        ('JZ', 10),
        ('LOAD', 0),
        ('PUSH', 1),
        ('SUB', ),
        ('STORE', 0),
        ('LOAD', 0),
        ('PUSH', 0),
        ('JZ', 18),
        ('PUSH', 1),
        ('JMP', 19),
        ('PUSH', 0),
        ('STORE', 3),  # result
    ])
    return vm.stack[3]


def warn_count(messagetype, command, conf, strict=None, ssh=None):
    conf['warning_counter'] = execute_vm_program()
    return 1, conf


def check_path(line, conf, completion=None, ssh=None, strict=None):
    vm = SimpleVM()
    vm.run([
        ('PUSH', 1),  # Initial return value
        ('STORE', 0),  # Return code
    ])
    
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

    relist = re.findall(r'[^=]\"(.+)\"', line)
    relist2 = re.findall(r'[^=]\'(.+)\'', line)
    relist = relist + relist2
    for item in relist:
        if os.path.exists(item):
            ret_check_path, conf = check_path(item, conf, strict=strict)
            returncode += ret_check_path

    line = re.sub(r'\"(.+?)\"', '', line)
    line = re.sub(r'\'(.+?)\'', '', line)

    if re.findall('[:cntrl:].*\n', line):
        ret, conf = warn_count('syntax', oline, conf, strict=strict, ssh=ssh)
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