# -*- coding: utf-8 -*-
from __future__ import unicode_literals, absolute_import
from collections import OrderedDict
import re

from io import StringIO

from configparser import ConfigParser, NoOptionError


from .regex import HASH_REGEX

from .dependencies import DependencyFile, Dependency
from packaging.requirements import Requirement as PackagingRequirement, InvalidRequirement
from . import filetypes
import toml
from packaging.specifiers import SpecifierSet
import json


def setuptools_parse_requirements_backport(strs):  
    def yield_lines(strs):
        if isinstance(strs, str):
            for s in strs.splitlines():
                s = s.strip()
                if s and not s.startswith('#'):
                    yield s
        else:
            for ss in strs:
                for s in yield_lines(ss):
                    yield s
    lines = iter(yield_lines(strs))

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
        dispatcher = 0
        lines_iter = self.iter_lines()
        while True:
            if dispatcher == 0:
                try:
                    line = next(lines_iter).rstrip()
                    dispatcher = 1
                except StopIteration:
                    break
            elif dispatcher == 1:
                if not line:
                    dispatcher = 0
                    continue
                elif line.startswith('#'):
                    dispatcher = 0
                    continue
                elif line.startswith('-i') or line.startswith('--index-url') or line.startswith('--extra-index-url'):
                    index_server = self.parse_index_server(line)
                    dispatcher = 0
                    continue
                elif self.obj.path and (line.startswith('-r') or line.startswith('--requirement')):
                    self.obj.resolved_files.append(self.resolve_file(self.obj.path, line))
                    dispatcher = 0
                    continue
                elif line.startswith('-f') or line.startswith('--find-links') or line.startswith('--no-index') or \
                        line.startswith('--allow-external') or line.startswith('--allow-unverified') or \
                        line.startswith('-Z') or line.startswith('--always-unzip'):
                    dispatcher = 0
                    continue
                elif self.is_marked_line(line):
                    dispatcher = 0
                    continue
                else:
                    dispatcher = 2
            elif dispatcher == 2:
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
                            dispatcher = 0
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
                    dispatcher = 0
                except ValueError:
                    dispatcher = 0
                    continue


class ToxINIParser(Parser):

    def parse(self):
        parser = ConfigParser()
        parser.readfp(StringIO(self.obj.content))
        dispatcher = 0
        sections_iter = iter(parser.sections())
        while True:
            if dispatcher == 0:
                try:
                    section = next(sections_iter)
                    dispatcher = 1
                except StopIteration:
                    break
            elif dispatcher == 1:
                try:
                    content = parser.get(section=section, option="deps")
                    dispatcher = 2
                except NoOptionError:
                    dispatcher = 0
            elif dispatcher == 2:
                lines_iter = enumerate(content.splitlines())
                for n, line in lines_iter:
                    if self.is_marked_line(line):
                        continue
                    if line:
                        req = RequirementsTXTLineParser.parse(line)
                        if req:
                            req.dependency_type = self.obj.file_type
                            self.obj.dependencies.append(req)
                dispatcher = 0


class CondaYMLParser(Parser):

    def parse(self):
        import yaml
        dispatcher = 0
        try:
            data = yaml.safe_load(self.obj.content)
            if data and 'dependencies' in data and isinstance(data['dependencies'], list):
                dependencies_iter = iter(data['dependencies'])
                dispatcher = 1
            else:
                dispatcher = -1
        except yaml.YAMLError:
            dispatcher = -1
        while dispatcher != -1:
            if dispatcher == 1:
                try:
                    dep = next(dependencies_iter)
                    dispatcher = 2
                except StopIteration:
                    break
            elif dispatcher == 2:
                if isinstance(dep, dict) and 'pip' in dep:
                    pip_iter = enumerate(dep['pip'])
                    dispatcher = 3
                else:
                    dispatcher = 1
            elif dispatcher == 3:
                try:
                    n, line = next(pip_iter)
                    if not self.is_marked_line(line):
                        req = RequirementsTXTLineParser.parse(line)
                        if req:
                            req.dependency_type = self.obj.file_type
                            self.obj.dependencies.append(req)
                except StopIteration:
                    dispatcher = 1


class PipfileParser(Parser):

    def parse(self):
        try:
            data = toml.loads(self.obj.content, _dict=OrderedDict)
            dispatcher = 0
        except (toml.TomlDecodeError, IndexError) as e:
            dispatcher = -1
        if dispatcher != -1:
            package_types_iter = iter(['packages', 'dev-packages'])
            while dispatcher != -1:
                if dispatcher == 0:
                    try:
                        package_type = next(package_types_iter)
                        dispatcher = 1
                    except StopIteration:
                        break
                elif dispatcher == 1:
                    if package_type in data:
                        items_iter = iter(data[package_type].items())
                        dispatcher = 2
                    else:
                        dispatcher = 0
                elif dispatcher == 2:
                    try:
                        name, specs = next(items_iter)
                        if isinstance(specs, str):
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
                    except StopIteration:
                        dispatcher = 0


class PipfileLockParser(Parser):

    def parse(self):
        try:
            data = json.loads(self.obj.content, object_pairs_hook=OrderedDict)
            dispatcher = 0
        except ValueError:
            dispatcher = -1
        if dispatcher != -1:
            package_types_iter = iter(['default', 'develop'])
            while dispatcher != -1:
                if dispatcher == 0:
                    try:
                        package_type = next(package_types_iter)
                        dispatcher = 1
                    except StopIteration:
                        break
                elif dispatcher == 1:
                    if package_type in data:
                        items_iter = iter(data[package_type].items())
                        dispatcher = 2
                    else:
                        dispatcher = 0
                elif dispatcher == 2:
                    try:
                        name, meta = next(items_iter)
                        if 'version' in meta:
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
                    except StopIteration:
                        dispatcher = 0


class SetupCfgParser(Parser):
    def parse(self):
        parser = ConfigParser()
        parser.readfp(StringIO(self.obj.content))
        dispatcher = 0
        sections_iter = iter(parser.values())
        while dispatcher != -1:
            if dispatcher == 0:
                try:
                    section = next(sections_iter)
                    dispatcher = 1
                except StopIteration:
                    break
            elif dispatcher == 1:
                if section.name == 'options':
                    options_iter = iter(['install_requires', 'setup_requires', 'test_require'])
                    dispatcher = 2
                elif section.name == 'options.extras_require':
                    values_iter = iter(section.values())
                    dispatcher = 4
                else:
                    dispatcher = 0
            elif dispatcher == 2:
                try:
                    name = next(options_iter)
                    content = section.get(name)
                    if content:
                        self._parse_content(content)
                except StopIteration:
                    dispatcher = 0
            elif dispatcher == 4:
                try:
                    content = next(values_iter)
                    self._parse_content(content)
                except StopIteration:
                    dispatcher = 0

    def _parse_content(self, content):
        lines_iter = enumerate(content.splitlines())
        for n, line in lines_iter:
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