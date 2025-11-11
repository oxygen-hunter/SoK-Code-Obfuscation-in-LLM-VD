import selinux
from stat import *
import gettext
translation = gettext.translation('setroubleshoot-plugins', fallback=True)
_ = translation.gettext

from setroubleshoot.util import *
from setroubleshoot.Plugin import Plugin
import subprocess

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.vars = {}

    def run(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            op = self.instructions[self.pc]
            getattr(self, f'op_{op[0]}')(*op[1:])
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.stack.append(a - b)

    def op_LOAD(self, var_name):
        self.stack.append(self.vars[var_name])

    def op_STORE(self, var_name):
        self.vars[var_name] = self.op_POP()

    def op_JMP(self, address):
        self.pc = address - 1

    def op_JZ(self, address):
        if self.op_POP() == 0:
            self.pc = address - 1

    def op_REPORT(self, type):
        if type == "unsafe":
            print("Unsafe report generated.")
        else:
            print("Safe report generated.")

class plugin(Plugin):
    def __init__(self):
        Plugin.__init__(self, __name__)
        self.set_priority(10)

    def analyze(self, avc):
        vm = VM()
        vm.run([
            ('PUSH', avc.tpath),
            ('STORE', 'tpath'),
            ('PUSH', 0),
            ('STORE', 'match_result'),
            ('LOAD', 'tpath'),
            ('PUSH', 'execmod'),
            ('CALL', self.has_any_access_in),
            ('JZ', 18),
            ('LOAD', 'tpath'),
            ('CALL', self.check_textrel),
            ('STORE', 'match_result'),
            ('LOAD', 'match_result'),
            ('JZ', 15),
            ('PUSH', 'unsafe'),
            ('REPORT',),
            ('JMP', 18),
            ('LOAD', 'tpath'),
            ('CALL', self.check_mcon),
            ('JZ', 18),
            ('PUSH', 'safe'),
            ('REPORT',),
        ])

    def has_any_access_in(self, tpath, access):
        return tpath in access

    def check_textrel(self, tpath):
        p1 = subprocess.Popen(['eu-readelf', '-d', tpath], stdout=subprocess.PIPE)
        p2 = subprocess.Popen(["fgrep", "-q", "TEXTREL"], stdin=p1.stdout, stdout=subprocess.PIPE)
        p1.stdout.close()
        p1.wait()
        p2.wait()
        return p2.returncode == 1

    def check_mcon(self, tpath):
        mcon = selinux.matchpathcon(tpath.strip('"'), S_IFREG)[1]
        return mcon.split(":")[2] == "lib_t"