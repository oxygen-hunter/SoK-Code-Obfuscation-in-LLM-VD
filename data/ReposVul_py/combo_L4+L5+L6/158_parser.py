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
    lines = iter(yield_lines(strs))

    def process_lines():
        try:
            line = next(lines)
            if ' #' in line:
                line = line[:line.find(' #')]
            if line.endswith('\\'):
                line = line[:-2].strip() + next(lines)
            yield PackagingRequirement(line)
            yield from process_lines()
        except StopIteration:
            return

    yield from process_lines()


class RequirementsTXTLineParser(object):

    @classmethod
    def parse(cls, line):
        try:
            parsed, = setuptools_parse_requirements_backport(line.replace("\t#", "\t #") if "\t#" in line else line)
        except InvalidRequirement:
            return None
        return Dependency(
            name=parsed.name,
            specs=parsed.specifier,
            line=line,
            extras=parsed.extras,
            dependency_type=filetypes.requirements_txt
        )


class Parser(object):

    def __init__(self, obj):
        self.obj = obj
        self._lines = None

    def iter_lines(self, lineno=0):
        def recursive_iter(lines, lineno):
            if lineno < len(lines):
                yield lines[lineno]
                yield from recursive_iter(lines, lineno + 1)
        yield from recursive_iter(self.lines, lineno)

    @property
    def lines(self):
        if self._lines is None:
            self._lines = self.obj.content.splitlines()
        return self._lines

    @property
    def is_marked_file(self):
        def recursive_check(lines, index, max_checks=2):
            if index >= min(max_checks, len(lines)):
                return False
            line = lines[index]
            for marker in self.obj.file_marker:
                if marker in line:
                    return True
            return recursive_check(lines, index + 1)

        return recursive_check(self.lines, 0)

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
        return groups[1] if len(groups) >= 2 and groups[1].endswith("/") else (groups[1] + "/" if len(groups) >= 2 else None)

    @classmethod
    def resolve_file(cls, file_path, line):
        line = line.replace("-r ", "").replace("--requirement ", "")
        parts = file_path.split("/")
        if " #" in line:
            line = line.split("#")[0].strip()
        return line if len(parts) == 1 else "/".join(parts[:-1]) + "/" + line


class RequirementsTXTParser(Parser):

    def parse(self):
        index_server = None

        def recursive_parse(lines, num):
            if num >= len(lines):
                return
            line = lines[num].rstrip()
            if not line or line.startswith('#'):
                recursive_parse(lines, num + 1)
                return

            if line.startswith('-i') or line.startswith('--index-url') or line.startswith('--extra-index-url'):
                nonlocal index_server
                index_server = self.parse_index_server(line)
            elif self.obj.path and (line.startswith('-r') or line.startswith('--requirement')):
                self.obj.resolved_files.append(self.resolve_file(self.obj.path, line))
            elif any(line.startswith(prefix) for prefix in ['-f', '--find-links', '--no-index', '--allow-external', '--allow-unverified', '-Z', '--always-unzip']):
                pass
            elif self.is_marked_line(line):
                pass
            else:
                try:
                    parseable_line = line.replace("\\", "")
                    if "\\" in line:
                        for next_line in self.iter_lines(num + 1):
                            parseable_line += next_line.strip().replace("\\", "")
                            line += "\n" + next_line
                            if "\\" not in next_line:
                                break
                        if self.is_marked_line(parseable_line):
                            recursive_parse(lines, num + 1)
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

                except ValueError:
                    pass

            recursive_parse(lines, num + 1)

        recursive_parse(self.lines, 0)


class ToxINIParser(Parser):

    def parse(self):
        parser = ConfigParser()
        parser.readfp(StringIO(self.obj.content))

        def recursive_section_parse(sections, index):
            if index >= len(sections):
                return
            section = sections[index]
            try:
                content = parser.get(section=section, option="deps")
                for line in content.splitlines():
                    if not self.is_marked_line(line) and line:
                        req = RequirementsTXTLineParser.parse(line)
                        if req:
                            req.dependency_type = self.obj.file_type
                            self.obj.dependencies.append(req)
            except NoOptionError:
                pass
            recursive_section_parse(sections, index + 1)

        recursive_section_parse(parser.sections(), 0)


class CondaYMLParser(Parser):

    def parse(self):
        import yaml
        try:
            data = yaml.safe_load(self.obj.content)
            if data and 'dependencies' in data and isinstance(data['dependencies'], list):
                def recursive_dependency_parse(dependencies, index):
                    if index >= len(dependencies):
                        return
                    dep = dependencies[index]
                    if isinstance(dep, dict) and 'pip' in dep:
                        for line in dep['pip']:
                            if not self.is_marked_line(line):
                                req = RequirementsTXTLineParser.parse(line)
                                if req:
                                    req.dependency_type = self.obj.file_type
                                    self.obj.dependencies.append(req)
                    recursive_dependency_parse(dependencies, index + 1)

                recursive_dependency_parse(data['dependencies'], 0)
        except yaml.YAMLError:
            pass


class PipfileParser(Parser):

    def parse(self):
        try:
            data = toml.loads(self.obj.content, _dict=OrderedDict)
            if data:
                def recursive_package_parse(package_types, index):
                    if index >= len(package_types):
                        return
                    package_type = package_types[index]
                    if package_type in data:
                        for name, specs in data[package_type].items():
                            if isinstance(specs, str) and specs != '*':
                                self.obj.dependencies.append(
                                    Dependency(
                                        name=name, specs=SpecifierSet(specs),
                                        dependency_type=filetypes.pipfile,
                                        line=''.join([name, specs]),
                                        section=package_type
                                    )
                                )
                    recursive_package_parse(package_types, index + 1)

                recursive_package_parse(['packages', 'dev-packages'], 0)
        except (toml.TomlDecodeError, IndexError):
            pass


class PipfileLockParser(Parser):

    def parse(self):
        try:
            data = json.loads(self.obj.content, object_pairs_hook=OrderedDict)
            if data:
                def recursive_lock_parse(package_types, index):
                    if index >= len(package_types):
                        return
                    package_type = package_types[index]
                    if package_type in data:
                        for name, meta in data[package_type].items():
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
                    recursive_lock_parse(package_types, index + 1)

                recursive_lock_parse(['default', 'develop'], 0)
        except ValueError:
            pass


class SetupCfgParser(Parser):
    def parse(self):
        parser = ConfigParser()
        parser.readfp(StringIO(self.obj.content))

        def recursive_section_parse(sections, index):
            if index >= len(sections):
                return
            section = sections[index]
            if section.name == 'options':
                options = 'install_requires', 'setup_requires', 'test_require'
                for name in options:
                    content = section.get(name)
                    if content:
                        self._parse_content(content)
            elif section.name == 'options.extras_require':
                for content in section.values():
                    self._parse_content(content)
            recursive_section_parse(sections, index + 1)

        recursive_section_parse(parser.values(), 0)

    def _parse_content(self, content):
        for line in content.splitlines():
            if not self.is_marked_line(line) and line:
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