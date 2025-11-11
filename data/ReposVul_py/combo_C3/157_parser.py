from __future__ import unicode_literals, absolute_import
from collections import OrderedDict
import re
import yaml
from io import StringIO
from configparser import ConfigParser, NoOptionError
from .regex import HASH_REGEX
from .dependencies import DependencyFile, Dependency
from packaging.requirements import Requirement as PackagingRequirement, InvalidRequirement
from . import filetypes
import toml
from packaging.specifiers import SpecifierSet
import json

# Virtual Machine instruction set
PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET = range(10)

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            op, *args = self.instructions[self.pc]
            self.pc += 1
            if op == PUSH:
                self.stack.append(args[0])
            elif op == POP:
                self.stack.pop()
            elif op == ADD:
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif op == SUB:
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a - b)
            elif op == JMP:
                self.pc = args[0]
            elif op == JZ:
                if self.stack.pop() == 0:
                    self.pc = args[0]
            elif op == LOAD:
                self.stack.append(args[0])
            elif op == STORE:
                var = self.stack.pop()
                self.stack.append(var)
            elif op == CALL:
                func = args[0]
                func()
            elif op == RET:
                return

def vm_parse_requirements(vm, strs):
    vm.load_instructions([
        (CALL, lambda: vm_yield_lines(vm, strs)),
        (RET,)
    ])
    vm.run()

def vm_yield_lines(vm, strs):
    vm.load_instructions([
        (LOAD, strs),
        (CALL, lambda: vm_yield_lines_logic(vm)),
        (RET,)
    ])
    vm.run()

def vm_yield_lines_logic(vm):
    def logic():
        strs = vm.stack.pop()
        if isinstance(strs, str):
            for s in strs.splitlines():
                s = s.strip()
                if s and not s.startswith('#'):
                    vm.stack.append(s)
        else:
            for ss in strs:
                vm.stack.append(ss)
                vm.load_instructions([(CALL, lambda: vm_yield_lines(vm, ss)), (RET,)])
                vm.run()
    logic()

def setuptools_parse_requirements_backport(strs):
    lines = []
    vm = VM()
    vm_parse_requirements(vm, strs)
    while vm.stack:
        lines.append(vm.stack.pop())

    for line in lines:
        if ' #' in line:
            line = line[:line.find(' #')]
        if line.endswith('\\'):
            line = line[:-2].strip()
            line += next(lines)
        yield PackagingRequirement(line)

class RequirementsTXTLineParser(object):
    @classmethod
    def parse(cls, line):
        try:
            if "\t#" in line:
                parsed, = setuptools_parse_requirements_backport(line.replace("\t#", "\t #"))
            else:
                parsed, = setuptools_parse_requirements_backport(line)
        except InvalidRequirement:
            return None
        dep = Dependency(
            name=parsed.name,
            specs=parsed.specifier,
            line=line,
            extras=parsed.extras,
            dependency_type=filetypes.requirements_txt
        )
        return dep

class Parser(object):
    def __init__(self, obj):
        self.obj = obj
        self._lines = None

    def iter_lines(self, lineno=0):
        for line in self.lines[lineno:]:
            yield line

    @property
    def lines(self):
        if self._lines is None:
            self._lines = self.obj.content.splitlines()
        return self._lines

    @property
    def is_marked_file(self):
        for n, line in enumerate(self.iter_lines()):
            for marker in self.obj.file_marker:
                if marker in line:
                    return True
            if n >= 2:
                break
        return False

    def is_marked_line(self, line):
        for marker in self.obj.line_marker:
            if marker in line:
                return True
        return False

    @classmethod
    def parse_hashes(cls, line):
        hashes = []
        for match in re.finditer(HASH_REGEX, line):
            hashes.append(line[match.start():match.end()])
        return re.sub(HASH_REGEX, "", line).strip(), hashes

    @classmethod
    def parse_index_server(cls, line):
        groups = re.split(pattern="[=\s]+", string=line.strip(), maxsplit=100)

        if len(groups) >= 2:
            return groups[1] if groups[1].endswith("/") else groups[1] + "/"

        return None

    @classmethod
    def resolve_file(cls, file_path, line):
        line = line.replace("-r ", "").replace("--requirement ", "")
        parts = file_path.split("/")
        if " #" in line:
            line = line.split("#")[0].strip()
        if len(parts) == 1:
            return line
        return "/".join(parts[:-1]) + "/" + line

class RequirementsTXTParser(Parser):
    def parse(self):
        index_server = None
        for num, line in enumerate(self.iter_lines()):
            line = line.rstrip()
            if not line:
                continue
            if line.startswith('#'):
                continue
            if line.startswith('-i') or \
                line.startswith('--index-url') or \
                line.startswith('--extra-index-url'):
                index_server = self.parse_index_server(line)
                continue
            elif self.obj.path and (line.startswith('-r') or line.startswith('--requirement')):
                self.obj.resolved_files.append(self.resolve_file(self.obj.path, line))
            elif line.startswith('-f') or line.startswith('--find-links') or \
                line.startswith('--no-index') or line.startswith('--allow-external') or \
                line.startswith('--allow-unverified') or line.startswith('-Z') or \
                line.startswith('--always-unzip'):
                continue
            elif self.is_marked_line(line):
                continue
            else:
                try:
                    parseable_line = line
                    if "\\" in line:
                        parseable_line = line.replace("\\", "")
                        for next_line in self.iter_lines(num + 1):
                            parseable_line += next_line.strip().replace("\\", "")
                            line += "\n" + next_line
                            if "\\" in next_line:
                                continue
                            break
                        if self.is_marked_line(parseable_line):
                            continue

                    hashes = []
                    if "--hash" in parseable_line:
                        parseable_line, hashes = Parser.parse_hashes(parseable_line)

                    req = RequirementsTXTLineParser.parse(parseable_line)
                    if req:
                        req.hashes = hashes
                        req.index_server = index_server
                        req.line = line
                        self.obj.dependencies.append(req)
                except ValueError:
                    continue

class ToxINIParser(Parser):
    def parse(self):
        parser = ConfigParser()
        parser.readfp(StringIO(self.obj.content))
        for section in parser.sections():
            try:
                content = parser.get(section=section, option="deps")
                for n, line in enumerate(content.splitlines()):
                    if self.is_marked_line(line):
                        continue
                    if line:
                        req = RequirementsTXTLineParser.parse(line)
                        if req:
                            req.dependency_type = self.obj.file_type
                            self.obj.dependencies.append(req)
            except NoOptionError:
                pass

class CondaYMLParser(Parser):
    def parse(self):
        try:
            data = yaml.safe_load(self.obj.content)
            if data and 'dependencies' in data and isinstance(data['dependencies'], list):
                for dep in data['dependencies']:
                    if isinstance(dep, dict) and 'pip' in dep:
                        for n, line in enumerate(dep['pip']):
                            if self.is_marked_line(line):
                                continue
                            req = RequirementsTXTLineParser.parse(line)
                            if req:
                                req.dependency_type = self.obj.file_type
                                self.obj.dependencies.append(req)
        except yaml.YAMLError:
            pass

class PipfileParser(Parser):
    def parse(self):
        try:
            data = toml.loads(self.obj.content, _dict=OrderedDict)
            if data:
                for package_type in ['packages', 'dev-packages']:
                    if package_type in data:
                        for name, specs in data[package_type].items():
                            if not isinstance(specs, str):
                                continue
                            if specs == '*':
                                specs = ''
                            self.obj.dependencies.append(
                                Dependency(
                                    name=name, specs=SpecifierSet(specs),
                                    dependency_type=filetypes.pipfile,
                                    line=''.join([name, specs]),
                                    section=package_type
                                )
                            )
        except (toml.TomlDecodeError, IndexError) as e:
            pass

class PipfileLockParser(Parser):
    def parse(self):
        try:
            data = json.loads(self.obj.content, object_pairs_hook=OrderedDict)
            if data:
                for package_type in ['default', 'develop']:
                    if package_type in data:
                        for name, meta in data[package_type].items():
                            if 'version' not in meta:
                                continue
                            specs = meta['version']
                            hashes = meta['hashes']
                            self.obj.dependencies.append(
                                Dependency(
                                    name=name, specs=SpecifierSet(specs),
                                    dependency_type=filetypes.pipfile_lock,
                                    hashes=hashes,
                                    line=''.join([name, specs]),
                                    section=package_type
                                )
                            )
        except ValueError:
            pass

class SetupCfgParser(Parser):
    def parse(self):
        parser = ConfigParser()
        parser.readfp(StringIO(self.obj.content))
        for section in parser.values():
            if section.name == 'options':
                options = 'install_requires', 'setup_requires', 'test_require'
                for name in options:
                    content = section.get(name)
                    if not content:
                        continue
                    self._parse_content(content)
            elif section.name == 'options.extras_require':
                for content in section.values():
                    self._parse_content(content)

    def _parse_content(self, content):
        for n, line in enumerate(content.splitlines()):
            if self.is_marked_line(line):
                continue
            if line:
                req = RequirementsTXTLineParser.parse(line)
                if req:
                    req.dependency_type = self.obj.file_type
                    self.obj.dependencies.append(req)

def parse(content, file_type=None, path=None, sha=None, marker=((), ()), parser=None):
    dep_file = DependencyFile(
        content=content,
        path=path,
        sha=sha,
        marker=marker,
        file_type=file_type,
        parser=parser
    )
    return dep_file.parse()