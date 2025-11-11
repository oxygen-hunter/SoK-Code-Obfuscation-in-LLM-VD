import gzip
import re
import secrets
import unicodedata
from gzip import GzipFile
from gzip import compress as gzip_compress
from io import BytesIO

from django.core.exceptions import SuspiciousFileOperation
from django.utils.functional import SimpleLazyObject, keep_lazy_text, lazy
from django.utils.regex_helper import _lazy_re_compile
from django.utils.translation import gettext as _
from django.utils.translation import gettext_lazy, pgettext

# VM instruction set
PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL = range(9)

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []
        self.memory = {}

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.pc < len(self.program):
            instr = self.program[self.pc]
            self.pc += 1
            getattr(self, f'instr_{instr[0]}')(*instr[1:])

    def instr_PUSH(self, value):
        self.stack.append(value)

    def instr_POP(self):
        self.stack.pop()

    def instr_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def instr_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def instr_JMP(self, target):
        self.pc = target

    def instr_JZ(self, target):
        if not self.stack.pop():
            self.pc = target

    def instr_LOAD(self, addr):
        self.stack.append(self.memory.get(addr, 0))

    def instr_STORE(self, addr):
        self.memory[addr] = self.stack.pop()

    def instr_CALL(self, func_index):
        self.call_functions[func_index]()

vm = VM()

# Function index map
functions = {
    "capfirst": 0,
    "wrap": 1,
    "add_truncation_text": 2,
    "get_valid_filename": 3,
    "get_text_list": 4,
    "normalize_newlines": 5,
    "phone2numeric": 6,
    "compress_string": 7,
    "unescape_string_literal": 8,
    "slugify": 9,
    "camel_case_to_spaces": 10,
}

def capfirst(x):
    vm.load_program([
        (PUSH, x),
        (PUSH, isinstance(x, str)),
        (JZ, 6),
        (LOAD, 0),
        (CALL, functions["capfirst"]),
        (ADD, ),
        (STORE, 0),
        (PUSH, vm.stack.pop()[0].upper()),
        (PUSH, vm.stack.pop()[1:]),
        (ADD, ),
    ])
    vm.run()
    return vm.stack.pop()

def wrap(text, width):
    def _generator():
        for line in text.splitlines(True):
            max_width = min((line.endswith("\n") and width + 1 or width), width)
            vm.stack.append((line, max_width))
            while len(line) > max_width:
                space = line[: max_width + 1].rfind(" ") + 1
                if space == 0:
                    space = line.find(" ") + 1
                    if space == 0:
                        yield line
                        line = ""
                        break
                yield "%s\n" % line[: space - 1]
                line = line[space:]
                max_width = min((line.endswith("\n") and width + 1 or width), width)
            if line:
                yield line

    return "".join(_generator())

def add_truncation_text(text, truncate=None):
    vm.load_program([
        (PUSH, truncate),
        (PUSH, text),
        (CALL, functions["add_truncation_text"]),
    ])
    vm.run()
    return vm.stack.pop()

@keep_lazy_text
def get_valid_filename(name):
    vm.load_program([
        (PUSH, name),
        (CALL, functions["get_valid_filename"]),
    ])
    vm.run()
    return vm.stack.pop()

@keep_lazy_text
def get_text_list(list_, last_word=gettext_lazy("or")):
    vm.load_program([
        (PUSH, list_),
        (PUSH, last_word),
        (CALL, functions["get_text_list"]),
    ])
    vm.run()
    return vm.stack.pop()

@keep_lazy_text
def normalize_newlines(text):
    vm.load_program([
        (PUSH, text),
        (CALL, functions["normalize_newlines"]),
    ])
    vm.run()
    return vm.stack.pop()

@keep_lazy_text
def phone2numeric(phone):
    vm.load_program([
        (PUSH, phone),
        (CALL, functions["phone2numeric"]),
    ])
    vm.run()
    return vm.stack.pop()

def compress_string(s, *, max_random_bytes=None):
    vm.load_program([
        (PUSH, s),
        (PUSH, max_random_bytes),
        (CALL, functions["compress_string"]),
    ])
    vm.run()
    return vm.stack.pop()

@keep_lazy_text
def unescape_string_literal(s):
    vm.load_program([
        (PUSH, s),
        (CALL, functions["unescape_string_literal"]),
    ])
    vm.run()
    return vm.stack.pop()

@keep_lazy_text
def slugify(value, allow_unicode=False):
    vm.load_program([
        (PUSH, value),
        (PUSH, allow_unicode),
        (CALL, functions["slugify"]),
    ])
    vm.run()
    return vm.stack.pop()

def camel_case_to_spaces(value):
    vm.load_program([
        (PUSH, value),
        (CALL, functions["camel_case_to_spaces"]),
    ])
    vm.run()
    return vm.stack.pop()

def _format_lazy(format_string, *args, **kwargs):
    return format_string.format(*args, **kwargs)

format_lazy = lazy(_format_lazy, str)