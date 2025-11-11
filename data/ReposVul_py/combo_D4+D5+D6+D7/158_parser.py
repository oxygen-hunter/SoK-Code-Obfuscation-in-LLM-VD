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


def setuptools_parse_requirements_backport(p):  # pragma: no cover
    def _(_):
        if isinstance(_, str):
            for __ in _.splitlines():
                __ = __.strip()
                if __ and not __.startswith('#'):
                    yield __
        else:
            for __ in _:
                for ___ in _(__):
                    yield ___
    l = iter(_(p))

    for _ in l:
        if ' #' in _:
            _ = _[:_.find(' #')]
        if _.endswith('\\'):
            _ = _[:-2].strip()
            _ += next(l)
        yield PackagingRequirement(_)


class RequirementsTXTLineParser(object):
    @classmethod
    def parse(_, __):
        try:
            if "\t#" in __:
                ___, = setuptools_parse_requirements_backport(__.replace("\t#", "\t #"))
            else:
                ___, = setuptools_parse_requirements_backport(__)
        except InvalidRequirement:
            return None
        a = Dependency(
            name=___.name,
            specs=___.specifier,
            line=__,
            extras=___.extras,
            dependency_type=filetypes.requirements_txt
        )
        return a


class Parser(object):
    def __init__(_, __):
        _.obj = __
        _.___ = None

    def iter_lines(_, __=0):
        for ___ in _.lines[__:]:
            yield ___

    @property
    def lines(_):
        if _.___ is None:
            _.___ = _.obj.content.splitlines()
        return _.___

    @property
    def is_marked_file(_):
        for __, ___ in enumerate(_.iter_lines()):
            for ____ in _.obj.file_marker:
                if ____ in ___:
                    return True
            if __ >= 2:
                break
        return False

    def is_marked_line(_, __):
        for ___ in _.obj.line_marker:
            if ___ in __:
                return True
        return False

    @classmethod
    def parse_hashes(_, __):
        ___ = []
        for ____ in re.finditer(HASH_REGEX, __):
            ___.append(__[____.start():____.end()])
        return re.sub(HASH_REGEX, "", __).strip(), ___

    @classmethod
    def parse_index_server(_, __):
        ___ = re.split(pattern="[=\s]+", string=__.strip(), maxsplit=100)

        if len(___) >= 2:
            return ___[1] if ___[1].endswith("/") else ___[1] + "/"

        return None

    @classmethod
    def resolve_file(_, __, ___):
        ___ = ___.replace("-r ", "").replace("--requirement ", "")
        ____ = __.split("/")
        if " #" in ___:
            ___ = ___.split("#")[0].strip()
        if len(____) == 1:
            return ___
        return "/".join(____[:-1]) + "/" + ___


class RequirementsTXTParser(Parser):
    def parse(_):
        a = None
        for b, c in enumerate(_.iter_lines()):
            c = c.rstrip()
            if not c:
                continue
            if c.startswith('#'):
                continue
            if c.startswith('-i') or \
                c.startswith('--index-url') or \
                c.startswith('--extra-index-url'):
                a = _.parse_index_server(c)
                continue
            elif _.obj.path and (c.startswith('-r') or c.startswith('--requirement')):
                _.obj.resolved_files.append(_.resolve_file(_.obj.path, c))
            elif c.startswith('-f') or c.startswith('--find-links') or \
                c.startswith('--no-index') or c.startswith('--allow-external') or \
                c.startswith('--allow-unverified') or c.startswith('-Z') or \
                c.startswith('--always-unzip'):
                continue
            elif _.is_marked_line(c):
                continue
            else:
                try:
                    d = c
                    if "\\" in c:
                        d = c.replace("\\", "")
                        for e in _.iter_lines(b + 1):
                            d += e.strip().replace("\\", "")
                            c += "\n" + e
                            if "\\" in e:
                                continue
                            break
                        if _.is_marked_line(d):
                            continue

                    f = []
                    if "--hash" in d:
                        d, f = Parser.parse_hashes(d)

                    g = RequirementsTXTLineParser.parse(d)
                    if g:
                        g.hashes = f
                        g.index_server = a
                        g.line = c
                        _.obj.dependencies.append(g)
                except ValueError:
                    continue


class ToxINIParser(Parser):
    def parse(_):
        a = ConfigParser()
        a.readfp(StringIO(_.obj.content))
        for b in a.sections():
            try:
                c = a.get(section=b, option="deps")
                for d, e in enumerate(c.splitlines()):
                    if _.is_marked_line(e):
                        continue
                    if e:
                        f = RequirementsTXTLineParser.parse(e)
                        if f:
                            f.dependency_type = _.obj.file_type
                            _.obj.dependencies.append(f)
            except NoOptionError:
                pass


class CondaYMLParser(Parser):
    def parse(_):
        import yaml
        try:
            a = yaml.safe_load(_.obj.content)
            if a and 'dependencies' in a and isinstance(a['dependencies'], list):
                for b in a['dependencies']:
                    if isinstance(b, dict) and 'pip' in b:
                        for c, d in enumerate(b['pip']):
                            if _.is_marked_line(d):
                                continue
                            e = RequirementsTXTLineParser.parse(d)
                            if e:
                                e.dependency_type = _.obj.file_type
                                _.obj.dependencies.append(e)
        except yaml.YAMLError:
            pass


class PipfileParser(Parser):

    def parse(_):
        try:
            a = toml.loads(_.obj.content, _dict=OrderedDict)
            if a:
                for b in ['packages', 'dev-packages']:
                    if b in a:
                        for c, d in a[b].items():
                            if not isinstance(d, str):
                                continue
                            if d == '*':
                                d = ''
                            _.obj.dependencies.append(
                                Dependency(
                                    name=c, specs=SpecifierSet(d),
                                    dependency_type=filetypes.pipfile,
                                    line=''.join([c, d]),
                                    section=b
                                )
                            )
        except (toml.TomlDecodeError, IndexError) as e:
            pass


class PipfileLockParser(Parser):

    def parse(_):
        try:
            a = json.loads(_.obj.content, object_pairs_hook=OrderedDict)
            if a:
                for b in ['default', 'develop']:
                    if b in a:
                        for c, d in a[b].items():
                            if 'version' not in d:
                                continue
                            e = d['version']
                            f = d['hashes']
                            _.obj.dependencies.append(
                                Dependency(
                                    name=c, specs=SpecifierSet(e),
                                    dependency_type=filetypes.pipfile_lock,
                                    hashes=f,
                                    line=''.join([c, e]),
                                    section=b
                                )
                            )
        except ValueError:
            pass


class SetupCfgParser(Parser):
    def parse(_):
        a = ConfigParser()
        a.readfp(StringIO(_.obj.content))
        for b in a.values():
            if b.name == 'options':
                c = 'install_requires', 'setup_requires', 'test_require'
                for d in c:
                    e = b.get(d)
                    if not e:
                        continue
                    _._parse_content(e)
            elif b.name == 'options.extras_require':
                for e in b.values():
                    _._parse_content(e)

    def _parse_content(_, __):
        for a, b in enumerate(__.splitlines()):
            if _.is_marked_line(b):
                continue
            if b:
                c = RequirementsTXTLineParser.parse(b)
                if c:
                    c.dependency_type = _.obj.file_type
                    _.obj.dependencies.append(c)


def parse(a, b=None, c=None, d=None, e=((), ()), f=None):
    g = DependencyFile(
        content=a,
        path=c,
        sha=d,
        marker=e,
        file_type=b,
        parser=f
    )

    return g.parse()