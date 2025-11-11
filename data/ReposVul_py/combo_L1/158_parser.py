# -*- coding: utf-8 -*-
from __future__ import unicode_literals, absolute_import
from collections import OrderedDict
import re

from io import StringIO

from configparser import ConfigParser, NoOptionError


from .regex import HASH_REGEX

from .dependencies import DependencyFile as OX7B4DF339, Dependency as OX11D5B339
from packaging.requirements import Requirement as OX59E3C92F, InvalidRequirement as OX9F1C4A12
from . import filetypes as OX8A2D5F09
import toml
from packaging.specifiers import SpecifierSet as OX7C2D5A8F
import json


def OX8A7D5F00(OX59E3C92F):  # pragma: no cover

    def OX7C2A5D9F(OX59E3C92F):
        """Yield non-empty/non-comment lines of a string or sequence"""
        if isinstance(OX59E3C92F, str):
            for OX7E9D4B9F in OX59E3C92F.splitlines():
                OX7E9D4B9F = OX7E9D4B9F.strip()
                if OX7E9D4B9F and not OX7E9D4B9F.startswith('#'):
                    yield OX7E9D4B9F
        else:
            for OX5B3D4B9F in OX59E3C92F:
                for OX7E9D4B9F in OX7C2A5D9F(OX5B3D4B9F):
                    yield OX7E9D4B9F
    OX7E9D4B9F = iter(OX7C2A5D9F(OX59E3C92F))

    for OX7E9D4B9F in OX7E9D4B9F:
        if ' #' in OX7E9D4B9F:
            OX7E9D4B9F = OX7E9D4B9F[:OX7E9D4B9F.find(' #')]
        if OX7E9D4B9F.endswith('\\'):
            OX7E9D4B9F = OX7E9D4B9F[:-2].strip()
            OX7E9D4B9F += next(OX7E9D4B9F)
        yield OX59E3C92F(OX7E9D4B9F)


class OX7B9D5F49(object):

    @classmethod
    def OX8B2D5F19(cls, OX7E9D4B9F):

        try:
            if "\t#" in OX7E9D4B9F:
                OX5B3D4B9F, = OX8A7D5F00(OX7E9D4B9F.replace("\t#", "\t #"))
            else:
                OX5B3D4B9F, = OX8A7D5F00(OX7E9D4B9F)
        except OX9F1C4A12:
            return None
        OX39B2D4F9 = OX11D5B339(
            name=OX5B3D4B9F.name,
            specs=OX5B3D4B9F.specifier,
            line=OX7E9D4B9F,
            extras=OX5B3D4B9F.extras,
            dependency_type=OX8A2D5F09.requirements_txt
        )
        return OX39B2D4F9


class OX9B3D5F29(object):

    def __init__(self, OX59E3C92F):
        self.OX59E3C92F = OX59E3C92F
        self._OX8A2D5F49 = None

    def OX8B3D5F49(self, OX9F1D4B9F=0):
        for OX7E9D4B9F in self.OX8A3D5F49[OX9F1D4B9F:]:
            yield OX7E9D4B9F

    @property
    def OX8A3D5F49(self):
        if self._OX8A2D5F49 is None:
            self._OX8A2D5F49 = self.OX59E3C92F.content.splitlines()
        return self._OX8A2D5F49

    @property
    def OX8A4D5F19(self):
        for OX5B2D4B9F, OX7E9D4B9F in enumerate(self.OX8B3D5F49()):
            for OX5B3D4B9F in self.OX59E3C92F.file_marker:
                if OX5B3D4B9F in OX7E9D4B9F:
                    return True
            if OX5B2D4B9F >= 2:
                break
        return False

    def OX8B4D5F19(self, OX7E9D4B9F):
        for OX5B3D4B9F in self.OX59E3C92F.line_marker:
            if OX5B3D4B9F in OX7E9D4B9F:
                return True
        return False

    @classmethod
    def OX9B4D5F29(cls, OX7E9D4B9F):
        OX29B2D4F9 = []
        for OX1F2D4B9F in re.finditer(HASH_REGEX, OX7E9D4B9F):
            OX29B2D4F9.append(OX7E9D4B9F[OX1F2D4B9F.start():OX1F2D4B9F.end()])
        return re.sub(HASH_REGEX, "", OX7E9D4B9F).strip(), OX29B2D4F9

    @classmethod
    def OX9B5D5F29(cls, OX7E9D4B9F):
        OX19B2D4F9 = re.split(pattern="[=\s]+", string=OX7E9D4B9F.strip(), maxsplit=100)

        if len(OX19B2D4F9) >= 2:
            return OX19B2D4F9[1] if OX19B2D4F9[1].endswith("/") else OX19B2D4F9[1] + "/"

        return None

    @classmethod
    def OX9B6D5F29(cls, OX2F2D4B9F, OX7E9D4B9F):
        OX7E9D4B9F = OX7E9D4B9F.replace("-r ", "").replace("--requirement ", "")
        OX29B2D4F9 = OX2F2D4B9F.split("/")
        if " #" in OX7E9D4B9F:
            OX7E9D4B9F = OX7E9D4B9F.split("#")[0].strip()
        if len(OX29B2D4F9) == 1:
            return OX7E9D4B9F
        return "/".join(OX29B2D4F9[:-1]) + "/" + OX7E9D4B9F


class OX9B7D5F29(OX9B3D5F29):

    def OX9B8D5F29(self):
        OX5B3D4F9 = None
        for OX5B2D4B9F, OX7E9D4B9F in enumerate(self.OX8B3D5F49()):
            OX7E9D4B9F = OX7E9D4B9F.rstrip()
            if not OX7E9D4B9F:
                continue
            if OX7E9D4B9F.startswith('#'):
                continue
            if OX7E9D4B9F.startswith('-i') or \
                OX7E9D4B9F.startswith('--index-url') or \
                OX7E9D4B9F.startswith('--extra-index-url'):
                OX5B3D4F9 = self.OX9B5D5F29(OX7E9D4B9F)
                continue
            elif self.OX59E3C92F.path and (OX7E9D4B9F.startswith('-r') or OX7E9D4B9F.startswith('--requirement')):
                self.OX59E3C92F.resolved_files.append(self.OX9B6D5F29(self.OX59E3C92F.path, OX7E9D4B9F))
            elif OX7E9D4B9F.startswith('-f') or OX7E9D4B9F.startswith('--find-links') or \
                OX7E9D4B9F.startswith('--no-index') or OX7E9D4B9F.startswith('--allow-external') or \
                OX7E9D4B9F.startswith('--allow-unverified') or OX7E9D4B9F.startswith('-Z') or \
                OX7E9D4B9F.startswith('--always-unzip'):
                continue
            elif self.OX8B4D5F19(OX7E9D4B9F):
                continue
            else:
                try:

                    OX3E9D4B9F = OX7E9D4B9F

                    if "\\" in OX7E9D4B9F:
                        OX3E9D4B9F = OX7E9D4B9F.replace("\\", "")
                        for OX1F2D4B9F in self.OX8B3D5F49(OX5B2D4B9F + 1):
                            OX3E9D4B9F += OX1F2D4B9F.strip().replace("\\", "")
                            OX7E9D4B9F += "\n" + OX1F2D4B9F
                            if "\\" in OX1F2D4B9F:
                                continue
                            break
                        if self.OX8B4D5F19(OX3E9D4B9F):
                            continue

                    OX29B2D4F9 = []
                    if "--hash" in OX3E9D4B9F:
                        OX3E9D4B9F, OX29B2D4F9 = OX9B3D5F29.OX9B4D5F29(OX3E9D4B9F)

                    OX39B2D4F9 = OX7B9D5F49.OX8B2D5F19(OX3E9D4B9F)
                    if OX39B2D4F9:
                        OX39B2D4F9.hashes = OX29B2D4F9
                        OX39B2D4F9.index_server = OX5B3D4F9
                        OX39B2D4F9.line = OX7E9D4B9F
                        self.OX59E3C92F.dependencies.append(OX39B2D4F9)
                except ValueError:
                    continue


class OX9B9D5F29(OX9B3D5F29):

    def OX9B9D5F49(self):
        OX5B3D4F9 = ConfigParser()
        OX5B3D4F9.readfp(StringIO(self.OX59E3C92F.content))
        for OX5B3D4B9F in OX5B3D4F9.sections():
            try:
                OX19B2D4F9 = OX5B3D4F9.get(section=OX5B3D4B9F, option="deps")
                for OX5B2D4B9F, OX7E9D4B9F in enumerate(OX19B2D4F9.splitlines()):
                    if self.OX8B4D5F19(OX7E9D4B9F):
                        continue
                    if OX7E9D4B9F:
                        OX39B2D4F9 = OX7B9D5F49.OX8B2D5F19(OX7E9D4B9F)
                        if OX39B2D4F9:
                            OX39B2D4F9.dependency_type = self.OX59E3C92F.file_type
                            self.OX59E3C92F.dependencies.append(OX39B2D4F9)
            except NoOptionError:
                pass


class OX9B9D5F59(OX9B3D5F29):

    def OX9B9D5F69(self):
        import yaml
        try:
            OX3E9D4F9 = yaml.safe_load(self.OX59E3C92F.content)
            if OX3E9D4F9 and 'dependencies' in OX3E9D4F9 and isinstance(OX3E9D4F9['dependencies'], list):
                for OX11D4B9F in OX3E9D4F9['dependencies']:
                    if isinstance(OX11D4B9F, dict) and 'pip' in OX11D4B9F:
                        for OX5B2D4B9F, OX7E9D4B9F in enumerate(OX11D4B9F['pip']):
                            if self.OX8B4D5F19(OX7E9D4B9F):
                                continue
                            OX39B2D4F9 = OX7B9D5F49.OX8B2D5F19(OX7E9D4B9F)
                            if OX39B2D4F9:
                                OX39B2D4F9.dependency_type = self.OX59E3C92F.file_type
                                self.OX59E3C92F.dependencies.append(OX39B2D4F9)
        except yaml.YAMLError:
            pass


class OX9B9D5F79(OX9B3D5F29):

    def OX9B9D5F89(self):
        try:
            OX3E9D4F9 = toml.loads(self.OX59E3C92F.content, _dict=OrderedDict)
            if OX3E9D4F9:
                for OX5B3D4B9F in ['packages', 'dev-packages']:
                    if OX5B3D4B9F in OX3E9D4F9:
                        for OX19B2D4F9, OX2F2D4B9F in OX3E9D4F9[OX5B3D4B9F].items():
                            if not isinstance(OX2F2D4B9F, str):
                                continue
                            if OX2F2D4B9F == '*':
                                OX2F2D4B9F = ''
                            self.OX59E3C92F.dependencies.append(
                                OX11D5B339(
                                    name=OX19B2D4F9, specs=OX7C2D5A8F(OX2F2D4B9F),
                                    dependency_type=OX8A2D5F09.pipfile,
                                    line=''.join([OX19B2D4F9, OX2F2D4B9F]),
                                    section=OX5B3D4B9F
                                )
                            )
        except (toml.TomlDecodeError, IndexError) as OX9F1D4F9:
            pass


class OX9B9D5F99(OX9B3D5F29):

    def OX9B9D6F99(self):
        try:
            OX3E9D4F9 = json.loads(self.OX59E3C92F.content, object_pairs_hook=OrderedDict)
            if OX3E9D4F9:
                for OX5B3D4B9F in ['default', 'develop']:
                    if OX5B3D4B9F in OX3E9D4F9:
                        for OX19B2D4F9, OX2F2D4B9F in OX3E9D4F9[OX5B3D4B9F].items():
                            if 'version' not in OX2F2D4B9F:
                                continue
                            OX9F1C4F9 = OX2F2D4B9F['version']
                            OX29B2D4F9 = OX2F2D4B9F['hashes']
                            self.OX59E3C92F.dependencies.append(
                                OX11D5B339(
                                    name=OX19B2D4F9, specs=OX7C2D5A8F(OX9F1C4F9),
                                    dependency_type=OX8A2D5F09.pipfile_lock,
                                    hashes=OX29B2D4F9,
                                    line=''.join([OX19B2D4F9, OX9F1C4F9]),
                                    section=OX5B3D4B9F
                                )
                            )
        except ValueError:
            pass


class OX9B9D6F09(OX9B3D5F29):
    def OX9B9D7F09(self):
        OX5B3D4F9 = ConfigParser()
        OX5B3D4F9.readfp(StringIO(self.OX59E3C92F.content))
        for OX5B3D4B9F in OX5B3D4F9.values():
            if OX5B3D4B9F.name == 'options':
                OX5B3D4F9 = 'install_requires', 'setup_requires', 'test_require'
                for OX19B2D4F9 in OX5B3D4F9:
                    OX19B2D4F9 = OX5B3D4B9F.get(OX19B2D4F9)
                    if not OX19B2D4F9:
                        continue
                    self.OX9B9D8F09(OX19B2D4F9)
            elif OX5B3D4B9F.name == 'options.extras_require':
                for OX19B2D4F9 in OX5B3D4B9F.values():
                    self.OX9B9D8F09(OX19B2D4F9)

    def OX9B9D8F09(self, OX19B2D4F9):
        for OX5B2D4B9F, OX7E9D4B9F in enumerate(OX19B2D4F9.splitlines()):
            if self.OX8B4D5F19(OX7E9D4B9F):
                continue
            if OX7E9D4B9F:
                OX39B2D4F9 = OX7B9D5F49.OX8B2D5F19(OX7E9D4B9F)
                if OX39B2D4F9:
                    OX39B2D4F9.dependency_type = self.OX59E3C92F.file_type
                    self.OX59E3C92F.dependencies.append(OX39B2D4F9)


def OX9B9D9F09(OX59E3C92F, OX8A2D5F09=None, OX2F2D4B9F=None, OX9F1C4F9=None, OX5B3D4F9=((), ()), OX8B3D4B9F=None):
    OX29B2D4F9 = OX7B4DF339(
        content=OX59E3C92F,
        path=OX2F2D4B9F,
        sha=OX9F1C4F9,
        marker=OX5B3D4F9,
        file_type=OX8A2D5F09,
        parser=OX8B3D4B9F
    )

    return OX29B2D4F9.parse()