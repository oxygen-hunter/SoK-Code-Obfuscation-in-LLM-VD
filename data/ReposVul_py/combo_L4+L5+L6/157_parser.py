# -*- coding: utf-8 -*-
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


def setuptools_parse_requirements_backport(strs):  # pragma: no cover
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
    lines = yield_lines(strs)

    def process_lines(iterator):
        try:
            line = next(iterator)
            if ' #' in line:
                line = line[:line.find(' #')]
            if line.endswith('\\'):
                line = line[:-2].strip()
                line += next(iterator)
            yield PackagingRequirement(line)
            yield from process_lines(iterator)
        except StopIteration:
            return

    yield from process_lines(lines)


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
        if lineno >= len(self.lines):
            return
        yield self.lines[lineno]
        yield from self.iter_lines(lineno + 1)

    @property
    def lines(self):
        if self._lines is None:
            self._lines = self.obj.content.splitlines()
        return self._lines

    @property
    def is_marked_file(self):
        def check_marker(n, iterator):
            if n >= 2:
                return False
            try:
                line = next(iterator)
                for marker in self.obj.file_marker:
                    if marker in line:
                        return True
                return check_marker(n + 1, iterator)
            except StopIteration:
                return False

        return check_marker(0, self.iter_lines())

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

        def process_lines(iterator):
            try:
                num, line = next(iterator)
                line = line.rstrip()
                if not line:
                    yield from process_lines(iterator)
                    return
                if line.startswith('#'):
                    yield from process_lines(iterator)
                    return
                if line.startswith('-i') or line.startswith('--index-url') or line.startswith('--extra-index-url'):
                    index_server = self.parse_index_server(line)
                    yield from process_lines(iterator)
                    return
                elif self.obj.path and (line.startswith('-r') or line.startswith('--requirement')):
                    self.obj.resolved_files.append(self.resolve_file(self.obj.path, line))
                    yield from process_lines(iterator)
                    return
                elif line.startswith('-f') or line.startswith('--find-links') or line.startswith('--no-index') or \
                        line.startswith('--allow-external') or line.startswith('--allow-unverified') or \
                        line.startswith('-Z') or line.startswith('--always-unzip'):
                    yield from process_lines(iterator)
                    return
                elif self.is_marked_line(line):
                    yield from process_lines(iterator)
                    return
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
                                yield from process_lines(iterator)
                                return

                        hashes = []
                        if "--hash" in parseable_line:
                            parseable_line, hashes = Parser.parse_hashes(parseable_line)

                        req = RequirementsTXTLineParser.parse(parseable_line)
                        if req:
                            req.hashes = hashes
                            req.index_server = index_server
                            req.line = line
                            self.obj.dependencies.append(req)
                        yield from process_lines(iterator)
                        return
                    except ValueError:
                        yield from process_lines(iterator)
                        return
            except StopIteration:
                return

        yield from process_lines(enumerate(self.iter_lines()))


class ToxINIParser(Parser):
    def parse(self):
        parser = ConfigParser()
        parser.readfp(StringIO(self.obj.content))

        def process_sections(iterator):
            try:
                section = next(iterator)
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
                yield from process_sections(iterator)
            except StopIteration:
                return

        yield from process_sections(iter(parser.sections()))


class CondaYMLParser(Parser):
    def parse(self):
        def process_dependencies(dependencies):
            if dependencies:
                dep = dependencies[0]
                if isinstance(dep, dict) and 'pip' in dep:
                    for n, line in enumerate(dep['pip']):
                        if self.is_marked_line(line):
                            continue
                        req = RequirementsTXTLineParser.parse(line)
                        if req:
                            req.dependency_type = self.obj.file_type
                            self.obj.dependencies.append(req)
                yield from process_dependencies(dependencies[1:])

        try:
            data = yaml.safe_load(self.obj.content)
            if data and 'dependencies' in data and isinstance(data['dependencies'], list):
                yield from process_dependencies(data['dependencies'])
        except yaml.YAMLError:
            pass


class PipfileParser(Parser):
    def parse(self):
        def process_package_type(packages):
            if packages:
                name, specs = packages[0]
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
                yield from process_package_type(packages[1:])

        try:
            data = toml.loads(self.obj.content, _dict=OrderedDict)
            if data:
                for package_type in ['packages', 'dev-packages']:
                    if package_type in data:
                        yield from process_package_type(list(data[package_type].items()))
        except (toml.TomlDecodeError, IndexError) as e:
            pass


class PipfileLockParser(Parser):
    def parse(self):
        def process_packages(packages):
            if packages:
                name, meta = packages[0]
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
                yield from process_packages(packages[1:])

        try:
            data = json.loads(self.obj.content, object_pairs_hook=OrderedDict)
            if data:
                for package_type in ['default', 'develop']:
                    if package_type in data:
                        yield from process_packages(list(data[package_type].items()))
        except ValueError:
            pass


class SetupCfgParser(Parser):
    def parse(self):
        parser = ConfigParser()
        parser.readfp(StringIO(self.obj.content))

        def process_sections(iterator):
            try:
                section = next(iterator)
                if section.name == 'options':
                    options = 'install_requires', 'setup_requires', 'test_require'
                    for name in options:
                        content = section.get(name)
                        if content:
                            self._parse_content(content)
                elif section.name == 'options.extras_require':
                    for content in section.values():
                        self._parse_content(content)
                yield from process_sections(iterator)
            except StopIteration:
                return

        yield from process_sections(iter(parser.values()))

    def _parse_content(self, content):
        def process_lines(lines):
            if lines:
                line = lines[0]
                if not self.is_marked_line(line) and line:
                    req = RequirementsTXTLineParser.parse(line)
                    if req:
                        req.dependency_type = self.obj.file_type
                        self.obj.dependencies.append(req)
                yield from process_lines(lines[1:])

        yield from process_lines(content.splitlines())


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