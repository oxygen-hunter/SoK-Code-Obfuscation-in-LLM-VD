# -*- coding: utf-8 -*-

import os
import re
import glob
from urllib import quote
import json

from twisted.web import static, resource, http

from Components.config import config
from Tools.Directories import fileExists
from utilities import lenient_force_utf_8, sanitise_filename_slashes


def new_getRequestHostname(self):
    host = self.getHeader(b'host')
    if host:
        if host[0] == '[':
            return host.split(']', 1)[0] + "]"
        return host.split(':', 1)[0].encode('ascii')
    return self.getHost().host.encode('ascii')


http.Request.getRequestHostname = new_getRequestHostname


class SimpleVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.memory = {}

    def load_program(self, program):
        self.instructions = program

    def run(self):
        while self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            getattr(self, f'op_{opcode}')(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        self.stack.pop()

    def op_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, addr):
        self.pc = addr - 1

    def op_JZ(self, addr):
        if self.stack.pop() == 0:
            self.pc = addr - 1

    def op_LOAD(self, addr):
        self.stack.append(self.memory.get(addr, 0))

    def op_STORE(self, addr):
        self.memory[addr] = self.stack.pop()

    def op_OUTPUT(self):
        print(self.stack[-1])


class FileController(resource.Resource):
    def render(self, request):
        vm = SimpleVM()
        program = self.compile_to_vm(request)
        vm.load_program(program)
        vm.run()
        return vm.stack[-1]

    def compile_to_vm(self, request):
        instructions = []
        if "action" in request.args:
            action = request.args["action"][0]
        else:
            action = "download"

        if "file" in request.args:
            filename = lenient_force_utf_8(request.args["file"][0])
            filename = sanitise_filename_slashes(os.path.realpath(filename))

            if not os.path.exists(filename):
                instructions.extend([
                    ("PUSH", f"File '{filename}' not found"),
                    ("OUTPUT",),
                ])
                return instructions

            if action == "stream":
                instructions.extend(self.handle_stream(request, filename))
            elif action == "delete":
                instructions.extend(self.handle_delete(request, filename))
            elif action == "download":
                instructions.extend(self.handle_download(request, filename))
            else:
                instructions.extend([
                    ("PUSH", "wrong action parameter"),
                    ("OUTPUT",),
                ])
        elif "dir" in request.args:
            instructions.extend(self.handle_dir(request))

        return instructions

    def handle_stream(self, request, filename):
        instructions = []
        name = "stream"
        if "name" in request.args:
            name = request.args["name"][0]

        port = config.OpenWebif.port.value
        proto = 'http'
        if request.isSecure():
            port = config.OpenWebif.https_port.value
            proto = 'https'
        ourhost = request.getHeader('host')
        m = re.match('.+\:(\d+)$', ourhost)
        if m is not None:
            port = m.group(1)

        response = "#EXTM3U\n#EXTVLCOPT--http-reconnect=true\n#EXTINF:-1,%s\n%s://%s:%s/file?action=download&file=%s" % (
            name, proto, request.getRequestHostname(), port, quote(filename))
        instructions.extend([
            ("PUSH", response),
            ("OUTPUT",),
        ])
        return instructions

    def handle_delete(self, request, filename):
        instructions = [
            ("PUSH", f"TODO: DELETE FILE: {filename}"),
            ("OUTPUT",),
        ]
        return instructions

    def handle_download(self, request, filename):
        instructions = [
            ("PUSH", static.File(filename, defaultType="application/octet-stream").render(request)),
            ("OUTPUT",),
        ]
        return instructions

    def handle_dir(self, request):
        instructions = []
        path = request.args["dir"][0]
        pattern = '*'
        data = []
        if "pattern" in request.args:
            pattern = request.args["pattern"][0]
        directories = []
        files = []
        if fileExists(path):
            try:
                files = glob.glob(path + '/' + pattern)
            except:
                files = []
            files.sort()
            tmpfiles = files[:]
            for x in tmpfiles:
                if os.path.isdir(x):
                    directories.append(x + '/')
                    files.remove(x)
            data.append({"result": True, "dirs": directories, "files": files})
        else:
            data.append({"result": False, "message": "path %s not exits" % (path)})
        instructions.extend([
            ("PUSH", json.dumps(data, indent=2)),
            ("OUTPUT",),
        ])
        return instructions