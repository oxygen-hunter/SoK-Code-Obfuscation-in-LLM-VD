# coding: utf-8

from __future__ import division, absolute_import

import io
import re
import codecs
import json
import yaml
from urllib.request import urlopen
from yaml import SafeLoader

import csv
from csv import register_dialect, Dialect, QUOTE_MINIMAL
from typing import List, Optional, Dict, Union, Sequence


class CrLfDialect(Dialect):
    delimiter = ','
    quotechar = '"'
    doublequote = True
    skipinitialspace = True
    lineterminator = '\r\n'
    quoting = QUOTE_MINIMAL
register_dialect("crlf", CrLfDialect)


class LfDialect(Dialect):
    delimiter = ','
    quotechar = '"'
    doublequote = True
    skipinitialspace = True
    lineterminator = '\n'
    quoting = QUOTE_MINIMAL
register_dialect("lf", LfDialect)


class MyDumper(yaml.SafeDumper):
    def increase_indent(self, flow=False, indentless=False):
        return super(MyDumper, self).increase_indent(flow, False)


def construct_yaml_str(self, node):
    return self.construct_scalar(node)

SafeLoader.add_constructor(u'tag:yaml.org,2002:str', construct_yaml_str)


class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr, *args = self.instructions[self.pc]
            getattr(self, f"op_{instr}")(*args)
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

    def op_JMP(self, addr):
        self.pc = addr - 1  # -1 to offset the pc increment in the loop

    def op_JZ(self, addr):
        if self.op_POP() == 0:
            self.pc = addr - 1

    def op_LOAD(self, var):
        self.op_PUSH(var)

    def op_STORE(self, var, index):
        var[index] = self.op_POP()


def replace_keys(d, keymap, force_snake_case):
    vm = VM()
    instructions = [
        ('PUSH', d),
        ('PUSH', keymap),
        ('PUSH', force_snake_case),
        ('LOAD', replace_keys_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def replace_keys_logic():
    d, keymap, force_snake_case = vm.stack[-3], vm.stack[-2], vm.stack[-1]
    return {
        to_snake(keymap.get(k, k)) if force_snake_case else keymap.get(k, k):
            v for k, v in d.items()
    }


def to_snake(value):
    vm = VM()
    instructions = [
        ('PUSH', value),
        ('LOAD', to_snake_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def to_snake_logic():
    value = vm.stack[-1]
    return re.sub(r'((?<!^)[A-Z])', "_\\1", value.strip('<>-')).lower().replace("-", "_")


def load_json(json_str):
    vm = VM()
    instructions = [
        ('PUSH', json_str),
        ('LOAD', load_json_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def load_json_logic():
    json_str = vm.stack[-1]
    return json.loads(json_str)


def load_jsonf(fpath, encoding):
    vm = VM()
    instructions = [
        ('PUSH', fpath),
        ('PUSH', encoding),
        ('LOAD', load_jsonf_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def load_jsonf_logic():
    fpath, encoding = vm.stack[-2], vm.stack[-1]
    with codecs.open(fpath, encoding=encoding) as f:
        return json.load(f)


def load_yaml(yaml_str):
    vm = VM()
    instructions = [
        ('PUSH', yaml_str),
        ('LOAD', load_yaml_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def load_yaml_logic():
    yaml_str = vm.stack[-1]
    return yaml.safe_load(yaml_str)


def load_yamlf(fpath, encoding):
    vm = VM()
    instructions = [
        ('PUSH', fpath),
        ('PUSH', encoding),
        ('LOAD', load_yamlf_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def load_yamlf_logic():
    fpath, encoding = vm.stack[-2], vm.stack[-1]
    with codecs.open(fpath, encoding=encoding) as f:
        return yaml.safe_load(f)


def load_csvf(fpath, fieldnames, encoding):
    vm = VM()
    instructions = [
        ('PUSH', fpath),
        ('PUSH', fieldnames),
        ('PUSH', encoding),
        ('LOAD', load_csvf_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def load_csvf_logic():
    fpath, fieldnames, encoding = vm.stack[-3], vm.stack[-2], vm.stack[-1]
    with open(fpath, mode='r', encoding=encoding) as f:
        snippet = f.read(8192)
        f.seek(0)

        dialect = csv.Sniffer().sniff(snippet)
        dialect.skipinitialspace = True
        return list(csv.DictReader(f, fieldnames=fieldnames, dialect=dialect))


def load_json_url(url):
    vm = VM()
    instructions = [
        ('PUSH', url),
        ('LOAD', load_json_url_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def load_json_url_logic():
    url = vm.stack[-1]
    return json.loads(urlopen(url).read())


def dump_csv(data, fieldnames, with_header=False, crlf=False):
    vm = VM()
    instructions = [
        ('PUSH', data),
        ('PUSH', fieldnames),
        ('PUSH', with_header),
        ('PUSH', crlf),
        ('LOAD', dump_csv_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def dump_csv_logic():
    data, fieldnames, with_header, crlf = vm.stack[-4], vm.stack[-3], vm.stack[-2], vm.stack[-1]

    def force_str(v):
        return dump_json(v).replace('"', "'") if isinstance(v, (dict, list)) else v

    with io.StringIO() as sio:
        dialect = 'crlf' if crlf else 'lf'
        writer = csv.DictWriter(sio, fieldnames=fieldnames, dialect=dialect, extrasaction='ignore')
        if with_header:
            writer.writeheader()
        for x in data:
            writer.writerow({k: force_str(v) for k, v in x.items()})
        sio.seek(0)
        return sio.read()


def save_csvf(data: list, fieldnames: Sequence[str], fpath: str, encoding: str, with_header=False, crlf=False) -> str:
    vm = VM()
    instructions = [
        ('PUSH', data),
        ('PUSH', fieldnames),
        ('PUSH', fpath),
        ('PUSH', encoding),
        ('PUSH', with_header),
        ('PUSH', crlf),
        ('LOAD', save_csvf_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def save_csvf_logic():
    data, fieldnames, fpath, encoding, with_header, crlf = vm.stack[-6], vm.stack[-5], vm.stack[-4], vm.stack[-3], vm.stack[-2], vm.stack[-1]
    with codecs.open(fpath, mode='w', encoding=encoding) as f:
        f.write(dump_csv(data, fieldnames, with_header=with_header, crlf=crlf))
        return fpath


def dump_json(data, indent=None):
    vm = VM()
    instructions = [
        ('PUSH', data),
        ('PUSH', indent),
        ('LOAD', dump_json_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def dump_json_logic():
    data, indent = vm.stack[-2], vm.stack[-1]
    return json.dumps(data,
                      indent=indent,
                      ensure_ascii=False,
                      sort_keys=True,
                      separators=(',', ': '))


def dump_yaml(data):
    vm = VM()
    instructions = [
        ('PUSH', data),
        ('LOAD', dump_yaml_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def dump_yaml_logic():
    data = vm.stack[-1]
    return yaml.dump(data,
                     indent=2,
                     encoding=None,
                     allow_unicode=True,
                     default_flow_style=False,
                     Dumper=MyDumper)


def save_yamlf(data: Union[list, dict], fpath: str, encoding: str) -> str:
    vm = VM()
    instructions = [
        ('PUSH', data),
        ('PUSH', fpath),
        ('PUSH', encoding),
        ('LOAD', save_yamlf_logic)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.op_POP()


def save_yamlf_logic():
    data, fpath, encoding = vm.stack[-3], vm.stack[-2], vm.stack[-1]
    with codecs.open(fpath, mode='w', encoding=encoding) as f:
        f.write(dump_yaml(data))
        return fpath