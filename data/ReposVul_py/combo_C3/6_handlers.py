import os
import mimetypes
import json
import base64

from tornado import web
from notebook.base.handlers import IPythonHandler

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
            op = instr[0]
            args = instr[1:]
            getattr(self, f'_{op.lower()}')(*args)
            self.pc += 1

    def _push(self, value):
        self.stack.append(value)

    def _pop(self):
        return self.stack.pop()

    def _load(self, register):
        self.stack.append(self.registers.get(register, None))

    def _store(self, register):
        self.registers[register] = self._pop()

    def _add(self):
        b = self._pop()
        a = self._pop()
        self.stack.append(a + b)

    def _sub(self):
        b = self._pop()
        a = self._pop()
        self.stack.append(a - b)

    def _jmp(self, address):
        self.pc = address - 1

    def _jz(self, address):
        if self._pop() == 0:
            self.pc = address - 1

    def _write(self):
        print(self._pop())

    def _set_header(self, header, value):
        self.registers[header] = value

    def _log_info(self, message):
        print(f"INFO: {message}")

    def _raise_error(self, code):
        raise web.HTTPError(code)

    def _serve_content(self, content):
        print(content)

class FilesHandler(IPythonHandler):
    def get(self, path):
        cm = self.contents_manager
        vm = VM()

        instructions = [
            ('PUSH', path),
            ('PUSH', '/'),
            ('PUSH', 0),
            ('SUB',),
            ('JZ', 10),
            ('LOAD', 'contents_manager'),
            ('LOAD', 'path'),
            ('CALL', 'is_hidden'),
            ('JZ', 10),
            ('LOG_INFO', "Refusing to serve hidden file, via 404 Error"),
            ('RAISE_ERROR', 404),
            ('PUSH', '/'),
            ('CALL', 'strip'),
            ('STORE', 'path'),
            ('LOAD', 'path'),
            ('PUSH', '/'),
            ('CALL', 'rsplit'),
            ('STORE', 'name'),
            ('LOAD', 'contents_manager'),
            ('LOAD', 'path'),
            ('PUSH', 'file'),
            ('CALL', 'get'),
            ('STORE', 'model'),
            ('LOAD', 'self'),
            ('PUSH', 'download'),
            ('CALL', 'get_argument'),
            ('JZ', 40),
            ('LOAD', 'name'),
            ('PUSH', 'attachment; filename="%s"'),
            ('CALL', 'set_header'),
            ('LOAD', 'name'),
            ('PUSH', '.ipynb'),
            ('CALL', 'endswith'),
            ('JZ', 50),
            ('PUSH', 'application/json'),
            ('CALL', 'set_header'),
            ('JMP', 60),
            ('LOAD', 'name'),
            ('CALL', 'guess_type'),
            ('STORE', 'cur_mime'),
            ('LOAD', 'cur_mime'),
            ('JZ', 70),
            ('CALL', 'set_header'),
            ('JMP', 80),
            ('LOAD', 'model'),
            ('PUSH', 'format'),
            ('CALL', '__getitem__'),
            ('PUSH', 'base64'),
            ('JZ', 90),
            ('PUSH', 'application/octet-stream'),
            ('CALL', 'set_header'),
            ('JMP', 100),
            ('PUSH', 'text/plain'),
            ('CALL', 'set_header'),
            ('LOAD', 'model'),
            ('PUSH', 'format'),
            ('CALL', '__getitem__'),
            ('PUSH', 'base64'),
            ('JZ', 110),
            ('LOAD', 'model'),
            ('PUSH', 'content'),
            ('CALL', '__getitem__'),
            ('CALL', 'encode'),
            ('STORE', 'b64_bytes'),
            ('LOAD', 'b64_bytes'),
            ('CALL', 'decodestring'),
            ('CALL', 'serve_content'),
            ('JMP', 120),
            ('LOAD', 'model'),
            ('PUSH', 'content'),
            ('CALL', '__getitem__'),
            ('CALL', 'dumps'),
            ('CALL', 'serve_content'),
            ('JMP', 130),
            ('LOAD', 'model'),
            ('PUSH', 'content'),
            ('CALL', '__getitem__'),
            ('CALL', 'serve_content'),
            ('CALL', 'flush')
        ]

        vm.load_program(instructions)
        vm.run()

default_handlers = [
    (r"/files/(.*)", FilesHandler),
]