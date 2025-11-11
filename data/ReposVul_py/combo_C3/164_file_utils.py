import codecs
import errno
import gzip
import os
import posixpath
import shutil
import sys
import tarfile
import tempfile
import stat

import urllib.parse
import urllib.request
from urllib.parse import unquote
from urllib.request import pathname2url

import yaml

try:
    from yaml import CSafeLoader as YamlSafeLoader, CSafeDumper as YamlSafeDumper
except ImportError:
    from yaml import SafeLoader as YamlSafeLoader, SafeDumper as YamlSafeDumper

from mlflow.entities import FileInfo
from mlflow.exceptions import MissingConfigException
from mlflow.utils.rest_utils import cloud_storage_http_request, augmented_raise_for_status

ENCODING = "utf-8"

class StackVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.labels = {}

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            self.pc += 1
            getattr(self, f"op_{opcode.lower()}")(*args)

    def op_push(self, value):
        self.stack.append(value)

    def op_pop(self):
        self.stack.pop()

    def op_add(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_sub(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_jmp(self, label):
        self.pc = self.labels[label]

    def op_jz(self, label):
        if self.stack.pop() == 0:
            self.pc = self.labels[label]

    def op_load(self, index):
        self.stack.append(self.stack[index])

    def op_store(self, index):
        self.stack[index] = self.stack.pop()

    def op_label(self, label):
        self.labels[label] = self.pc

# Compile original functions into VM instructions
def is_directory(name):
    vm = StackVM()
    vm.load_program([
        ("PUSH", name),
        ("PUSH", os.path.isdir),
        ("LOAD", 0),
        ("CALL", 1),
        ("RET",)
    ])
    vm.run()
    return vm.stack.pop()

def is_file(name):
    vm = StackVM()
    vm.load_program([
        ("PUSH", name),
        ("PUSH", os.path.isfile),
        ("LOAD", 0),
        ("CALL", 1),
        ("RET",)
    ])
    vm.run()
    return vm.stack.pop()

def exists(name):
    vm = StackVM()
    vm.load_program([
        ("PUSH", name),
        ("PUSH", os.path.exists),
        ("LOAD", 0),
        ("CALL", 1),
        ("RET",)
    ])
    vm.run()
    return vm.stack.pop()

def list_all(root, filter_func=lambda x: True, full_path=False):
    vm = StackVM()
    vm.load_program([
        ("PUSH", root),
        ("PUSH", is_directory),
        ("LOAD", 0),
        ("CALL", 1),
        ("JZ", "invalid_dir"),
        ("PUSH", os.listdir(root)),
        ("PUSH", filter_func),
        ("CALL", 1),
        ("PUSH", [os.path.join(root, x) for x in os.listdir(root)]),
        ("STORE", 0),
        ("LABEL", "invalid_dir"),
        ("PUSH", []),
        ("RET",)
    ])
    vm.run()
    return vm.stack.pop()

def list_subdirs(dir_name, full_path=False):
    vm = StackVM()
    vm.load_program([
        ("PUSH", dir_name),
        ("PUSH", os.path.isdir),
        ("CALL", 1),
        ("JZ", "end"),
        ("PUSH", list_all),
        ("LOAD", 0),
        ("CALL", 1),
        ("LABEL", "end"),
        ("RET",)
    ])
    vm.run()
    return vm.stack.pop()

def list_files(dir_name, full_path=False):
    vm = StackVM()
    vm.load_program([
        ("PUSH", dir_name),
        ("PUSH", os.path.isfile),
        ("CALL", 1),
        ("JZ", "end"),
        ("PUSH", list_all),
        ("LOAD", 0),
        ("CALL", 1),
        ("LABEL", "end"),
        ("RET",)
    ])
    vm.run()
    return vm.stack.pop()

def find(root, name, full_path=False):
    vm = StackVM()
    vm.load_program([
        ("PUSH", root),
        ("PUSH", name),
        ("PUSH", os.path.join(root, name)),
        ("PUSH", list_all),
        ("LOAD", 0),
        ("CALL", 1),
        ("RET",)
    ])
    vm.run()
    return vm.stack.pop()

# You can continue to convert other functions similarly...