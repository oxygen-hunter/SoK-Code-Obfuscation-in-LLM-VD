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


def setuptools_parse_requirements_backport(v):  # pragma: no cover
    def yield_lines(x):
        if isinstance(x, str):
            for s in x.splitlines():
                s = s.strip()
                if s and not s.startswith('#'):
                    yield s
        else:
            for ss in x:
                for s in yield_lines(ss):
                    yield s
    lines = iter(yield_lines(v))

    for l in lines:
        if ' #' in l:
            l = l[:l.find(' #')]
        if l.endswith('\\'):
            l = l[:-2].strip()
            l += next(lines)
        yield PackagingRequirement(l)


class RequirementsTXTLineParser(object):
    @classmethod
    def parse(cls, v):
        try:
            if "\t#" in v:
                parsed, = setuptools_parse_requirements_backport(v.replace("\t#", "\t #"))
            else:
                parsed, = setuptools_parse_requirements_backport(v)
        except InvalidRequirement:
            return None
        d = Dependency(
            name=parsed.name,
            specs=parsed.specifier,
            line=v,
            extras=parsed.extras,
            dependency_type=filetypes.requirements_txt
        )
        return d


class Parser(object):
    def __init__(self, obj):
        self.obj = obj
        self._data = None

    def iter_lines(self, v=0):
        for l in self.lines[v:]:
            yield l

    @property
    def lines(self):
        if self._data is None:
            self._data = self.obj.content.splitlines()
        return self._data

    @property
    def is_marked_file(self):
        for n, l in enumerate(self.iter_lines()):
            for m in self.obj.file_marker:
                if m in l:
                    return True
            if n >= 2:
                break
        return False

    def is_marked_line(self, l):
        for m in self.obj.line_marker:
            if m in l:
                return True
        return False

    @classmethod
    def parse_hashes(cls, v):
        h = []
        for m in re.finditer(HASH_REGEX, v):
            h.append(v[m.start():m.end()])
        return re.sub(HASH_REGEX, "", v).strip(), h

    @classmethod
    def parse_index_server(cls, v):
        g = re.split(pattern="[=\s]+", string=v.strip(), maxsplit=100)

        if len(g) >= 2:
            return g[1] if g[1].endswith("/") else g[1] + "/"

        return None

    @classmethod
    def resolve_file(cls, p, v):
        v = v.replace("-r ", "").replace("--requirement ", "")
        p = p.split("/")
        if " #" in v:
            v = v.split("#")[0].strip()
        if len(p) == 1:
            return v
        return "/".join(p[:-1]) + "/" + v


class RequirementsTXTParser(Parser):
    def parse(self):
        i = None
        for n, l in enumerate(self.iter_lines()):
            l = l.rstrip()
            if not l:
                continue
            if l.startswith('#'):
                continue
            if l.startswith('-i') or \
                l.startswith('--index-url') or \
                l.startswith('--extra-index-url'):
                i = self.parse_index_server(l)
                continue
            elif self.obj.path and (l.startswith('-r') or l.startswith('--requirement')):
                self.obj.resolved_files.append(self.resolve_file(self.obj.path, l))
            elif l.startswith('-f') or l.startswith('--find-links') or \
                l.startswith('--no-index') or l.startswith('--allow-external') or \
                l.startswith('--allow-unverified') or l.startswith('-Z') or \
                l.startswith('--always-unzip'):
                continue
            elif self.is_marked_line(l):
                continue
            else:
                try:
                    p = l

                    if "\\" in l:
                        p = l.replace("\\", "")
                        for next_line in self.iter_lines(n + 1):
                            p += next_line.strip().replace("\\", "")
                            l += "\n" + next_line
                            if "\\" in next_line:
                                continue
                            break
                        if self.is_marked_line(p):
                            continue

                    h = []
                    if "--hash" in p:
                        p, h = Parser.parse_hashes(p)

                    r = RequirementsTXTLineParser.parse(p)
                    if r:
                        r.hashes = h
                        r.index_server = i
                        r.line = l
                        self.obj.dependencies.append(r)
                except ValueError:
                    continue


class ToxINIParser(Parser):
    def parse(self):
        p = ConfigParser()
        p.readfp(StringIO(self.obj.content))
        for s in p.sections():
            try:
                c = p.get(section=s, option="deps")
                for n, l in enumerate(c.splitlines()):
                    if self.is_marked_line(l):
                        continue
                    if l:
                        r = RequirementsTXTLineParser.parse(l)
                        if r:
                            r.dependency_type = self.obj.file_type
                            self.obj.dependencies.append(r)
            except NoOptionError:
                pass


class CondaYMLParser(Parser):
    def parse(self):
        try:
            d = yaml.safe_load(self.obj.content)
            if d and 'dependencies' in d and isinstance(d['dependencies'], list):
                for dep in d['dependencies']:
                    if isinstance(dep, dict) and 'pip' in dep:
                        for n, l in enumerate(dep['pip']):
                            if self.is_marked_line(l):
                                continue
                            r = RequirementsTXTLineParser.parse(l)
                            if r:
                                r.dependency_type = self.obj.file_type
                                self.obj.dependencies.append(r)
        except yaml.YAMLError:
            pass


class PipfileParser(Parser):

    def parse(self):
        try:
            d = toml.loads(self.obj.content, _dict=OrderedDict)
            if d:
                for pt in ['packages', 'dev-packages']:
                    if pt in d:
                        for n, s in d[pt].items():
                            if not isinstance(s, str):
                                continue
                            if s == '*':
                                s = ''
                            self.obj.dependencies.append(
                                Dependency(
                                    name=n, specs=SpecifierSet(s),
                                    dependency_type=filetypes.pipfile,
                                    line=''.join([n, s]),
                                    section=pt
                                )
                            )
        except (toml.TomlDecodeError, IndexError) as e:
            pass


class PipfileLockParser(Parser):

    def parse(self):
        try:
            d = json.loads(self.obj.content, object_pairs_hook=OrderedDict)
            if d:
                for pt in ['default', 'develop']:
                    if pt in d:
                        for n, m in d[pt].items():
                            if 'version' not in m:
                                continue
                            s = m['version']
                            h = m['hashes']
                            self.obj.dependencies.append(
                                Dependency(
                                    name=n, specs=SpecifierSet(s),
                                    dependency_type=filetypes.pipfile_lock,
                                    hashes=h,
                                    line=''.join([n, s]),
                                    section=pt
                                )
                            )
        except ValueError:
            pass


class SetupCfgParser(Parser):
    def parse(self):
        p = ConfigParser()
        p.readfp(StringIO(self.obj.content))
        for s in p.values():
            if s.name == 'options':
                o = 'install_requires', 'setup_requires', 'test_require'
                for n in o:
                    c = s.get(n)
                    if not c:
                        continue
                    self._parse_content(c)
            elif s.name == 'options.extras_require':
                for c in s.values():
                    self._parse_content(c)

    def _parse_content(self, c):
        for n, l in enumerate(c.splitlines()):
            if self.is_marked_line(l):
                continue
            if l:
                r = RequirementsTXTLineParser.parse(l)
                if r:
                    r.dependency_type = self.obj.file_type
                    self.obj.dependencies.append(r)


def parse(c, f=None, p=None, s=None, m=((), ()), parser=None):
    df = DependencyFile(
        content=c,
        path=p,
        sha=s,
        marker=m,
        file_type=f,
        parser=parser
    )

    return df.parse()