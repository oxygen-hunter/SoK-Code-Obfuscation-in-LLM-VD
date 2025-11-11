__author__ = "Gina Häußge <osd@foosel.net>"
__license__ = "GNU Affero General Public License http://www.gnu.org/licenses/agpl.html"
__copyright__ = "Copyright (C) 2015 The OctoPrint Project - Released under terms of the AGPLv3 License"

import logging
import os
import tarfile
import zipfile
from collections import defaultdict

from flask import abort, jsonify, request
from flask_babel import Locale

from octoprint.access.permissions import Permissions
from octoprint.plugin import plugin_manager
from octoprint.server.api import api
from octoprint.server.util.flask import no_firstrun_access
from octoprint.settings import settings
from octoprint.util import yaml

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.running = True
        self.instructions = {
            'PUSH': self.push,
            'POP': self.pop,
            'LOAD': self.load,
            'STORE': self.store,
            'JMP': self.jmp,
            'JZ': self.jz,
            'CALL': self.call,
            'RET': self.ret
        }
        self.memory = {}

    def push(self, value):
        self.stack.append(value)
        self.pc += 1

    def pop(self):
        self.stack.pop()
        self.pc += 1

    def load(self, address):
        self.stack.append(self.memory.get(address, 0))
        self.pc += 1

    def store(self, address):
        self.memory[address] = self.stack.pop()
        self.pc += 1

    def jmp(self, address):
        self.pc = address

    def jz(self, address):
        if self.stack.pop() == 0:
            self.pc = address
        else:
            self.pc += 1

    def call(self, address):
        self.stack.append(self.pc + 1)
        self.pc = address

    def ret(self):
        self.pc = self.stack.pop()

    def execute(self, bytecode):
        while self.running and self.pc < len(bytecode):
            instruction = bytecode[self.pc]
            op = instruction[0]
            args = instruction[1:]
            self.instructions[op](*args)

def get_installed_language_packs(vm):
    vm.execute([
        ('LOAD', 'translation_folder'),
        ('CALL', 10),
        ('JMP', 99),
        (10, 'PUSH', os.path.exists(vm.memory['translation_folder'])),
        ('JZ', 20),
        ('PUSH', {'_core': []}),
        ('STORE', 'result'),
        ('RET',),
        (20, 'PUSH', []),
        ('STORE', 'core_packs'),
        ('PUSH', defaultdict(lambda: {"identifier": None, "display": None, "languages": []})),
        ('STORE', 'plugin_packs'),
        ('LOAD', 'translation_folder'),
        ('CALL', 30),
        ('STORE', 'entries'),
        ('LOAD', 'entries'),
        ('CALL', 40),
        ('STORE', 'result'),
        ('RET',),
        (30, 'PUSH', lambda d: [e for e in os.scandir(d) if e.is_dir()]),
        ('RET',),
        (40, 'PUSH', lambda entries: {
            "_core": {"identifier": "_core", "display": "Core", "languages": core_packs},
            **plugin_packs
        }),
        ('RET',),
        (99, 'LOAD', 'result'),
        ('CALL', 50),
        ('RET',),
        (50, 'PUSH', lambda r: jsonify(language_packs=r)),
        ('RET',)
    ])

def upload_language_pack(vm):
    vm.execute([
        ('LOAD', 'input_upload_path'),
        ('LOAD', 'request.values'),
        ('CALL', 60),
        ('JZ', 70),
        ('LOAD', 'input_upload_name'),
        ('LOAD', 'request.values'),
        ('CALL', 60),
        ('JZ', 70),
        ('LOAD', 'request.values'),
        ('LOAD', 'input_upload_name'),
        ('STORE', 'upload_name'),
        ('LOAD', 'request.values'),
        ('LOAD', 'input_upload_path'),
        ('STORE', 'upload_path'),
        ('LOAD', 'upload_name'),
        ('CALL', 80),
        ('JZ', 90),
        ('LOAD', 'upload_path'),
        ('CALL', 100),
        ('RET',),
        (60, 'PUSH', lambda d, k: d.get(k) is not None),
        ('RET',),
        (70, 'PUSH', abort(400, description="No file included")),
        ('RET',),
        (80, 'PUSH', lambda name: any(name.lower().endswith(x) for x in (".zip", ".tar.gz", ".tgz", ".tar"))),
        ('RET',),
        (90, 'PUSH', abort(400, description="Invalid extension")),
        ('RET',),
        (100, 'PUSH', lambda path: tarfile.is_tarfile(path) or zipfile.is_zipfile(path)),
        ('RET',)
    ])

def delete_installed_language_pack(vm):
    vm.execute([
        ('LOAD', 'pack'),
        ('PUSH', '_core'),
        ('CALL', 110),
        ('JMP', 120),
        (110, 'PUSH', lambda p1, p2: os.path.join(settings().getBaseFolder("translations"), p1) if p1 == p2 else os.path.join(settings().getBaseFolder("translations"), "_plugins", p2, p1)),
        ('STORE', 'target_path'),
        ('RET',),
        (120, 'LOAD', 'target_path'),
        ('CALL', 130),
        ('RET',),
        (130, 'PUSH', lambda d: os.path.isdir(d) and shutil.rmtree(d)),
        ('RET',)
    ])

def _unpack_uploaded_zipfile(vm):
    vm.execute([
        ('LOAD', 'zipfile.ZipFile'),
        ('LOAD', 'path'),
        ('CALL', 140),
        ('STORE', 'zip'),
        ('LOAD', 'zip'),
        ('CALL', 150),
        ('RET',),
        (140, 'PUSH', lambda z, p: z(p, "r")),
        ('RET',),
        (150, 'PUSH', lambda z: z.extractall(target)),
        ('RET',)
    ])

def _unpack_uploaded_tarball(vm):
    vm.execute([
        ('LOAD', 'tarfile.open'),
        ('LOAD', 'path'),
        ('CALL', 160),
        ('STORE', 'tar'),
        ('LOAD', 'tar'),
        ('CALL', 170),
        ('RET',),
        (160, 'PUSH', lambda t, p: t(p, "r")),
        ('RET',),
        (170, 'PUSH', lambda t: t.extractall(target)),
        ('RET',)
    ])

def _validate_archive_name(vm):
    vm.execute([
        ('LOAD', 'name'),
        ('CALL', 180),
        ('JZ', 190),
        ('RET',),
        (180, 'PUSH', lambda n: os.path.abspath(os.path.join(target, n)).startswith(target + os.path.sep)),
        ('RET',),
        (190, 'PUSH', InvalidLanguagePack(f"Invalid name {name}")),
        ('RET',)
    ])

class InvalidLanguagePack(Exception):
    pass

vm = VM()
vm.memory['translation_folder'] = settings().getBaseFolder("translations", check_writable=False)
get_installed_language_packs(vm)
upload_language_pack(vm)
delete_installed_language_pack(vm)
_unpack_uploaded_zipfile(vm)
_unpack_uploaded_tarball(vm)
_validate_archive_name(vm)