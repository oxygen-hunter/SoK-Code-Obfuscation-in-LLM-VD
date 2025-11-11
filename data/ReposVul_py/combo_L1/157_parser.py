# -*- coding: utf-8 -*-
from __future__ import unicode_literals, absolute_import
from collections import OrderedDict
import re
import yaml

from io import StringIO

from configparser import ConfigParser, NoOptionError


from .regex import HASH_REGEX

from .dependencies import DependencyFile as OX0E4F0C8B, Dependency as OX7E1D9C2A
from packaging.requirements import Requirement as OXF3A1D7B3, InvalidRequirement as OX0D59FB4B
from . import filetypes as OX1C376A0B
import toml
from packaging.specifiers import SpecifierSet as OX1F5F4B12
import json


def OX1B2D3E4F(OX2A3B4C5D):  # pragma: no cover
    def OX3C4D5E6F(OX4D5E6F7A):
        if isinstance(OX4D5E6F7A, str):
            for OX5E6F7A8B in OX4D5E6F7A.splitlines():
                OX5E6F7A8B = OX5E6F7A8B.strip()
                if OX5E6F7A8B and not OX5E6F7A8B.startswith('#'):
                    yield OX5E6F7A8B
        else:
            for OX6F7A8B9C in OX4D5E6F7A:
                for OX7A8B9C0D in OX3C4D5E6F(OX6F7A8B9C):
                    yield OX7A8B9C0D
    OX8B9C0D1E = iter(OX3C4D5E6F(OX2A3B4C5D))

    for OX9C0D1E2F in OX8B9C0D1E:
        if ' #' in OX9C0D1E2F:
            OX9C0D1E2F = OX9C0D1E2F[:OX9C0D1E2F.find(' #')]
        if OX9C0D1E2F.endswith('\\'):
            OX9C0D1E2F = OX9C0D1E2F[:-2].strip()
            OX9C0D1E2F += next(OX8B9C0D1E)
        yield OXF3A1D7B3(OX9C0D1E2F)


class OXA1B2C3D4(object):
    @classmethod
    def OXB2C3D4E5(cls, OXA2B3C4D5):
        try:
            if "\t#" in OXA2B3C4D5:
                OX3D4E5F6G, = OX1B2D3E4F(OXA2B3C4D5.replace("\t#", "\t #"))
            else:
                OX3D4E5F6G, = OX1B2D3E4F(OXA2B3C4D5)
        except OX0D59FB4B:
            return None
        OXC4D5E6F7 = OX7E1D9C2A(
            name=OX3D4E5F6G.name,
            specs=OX3D4E5F6G.specifier,
            line=OXA2B3C4D5,
            extras=OX3D4E5F6G.extras,
            dependency_type=OX1C376A0B.requirements_txt
        )
        return OXC4D5E6F7


class OXA3B4C5D6(object):
    def __init__(self, OXC5D6E7F8):
        self.OX5F6G7H8I = OXC5D6E7F8
        self._OX6G7H8I9J = None

    def OXA4B5C6D7(self, OXA5B6C7D8=0):
        for OX8I9J0K1L in self.OX7H8I9J0K[OXA5B6C7D8:]:
            yield OX8I9J0K1L

    @property
    def OX7H8I9J0K(self):
        if self._OX6G7H8I9J is None:
            self._OX6G7H8I9J = self.OX5F6G7H8I.content.splitlines()
        return self._OX6G7H8I9J

    @property
    def OXA5B6C7D8(self):
        for OX9J0K1L2M, OXL2M3N4O5 in enumerate(self.OXA4B5C6D7()):
            for OX1L2M3N4O in self.OX5F6G7H8I.file_marker:
                if OX1L2M3N4O in OXL2M3N4O5:
                    return True
            if OX9J0K1L2M >= 2:
                break
        return False

    def OXB5C6D7E8(self, OXA6B7C8D9):
        for OX2M3N4O5P in self.OX5F6G7H8I.line_marker:
            if OX2M3N4O5P in OXA6B7C8D9:
                return True
        return False

    @classmethod
    def OXC6D7E8F9(cls, OXA7B8C9D0):
        OX3N4O5P6Q = []
        for OX4N5O6P7Q in re.finditer(HASH_REGEX, OXA7B8C9D0):
            OX3N4O5P6Q.append(OXA7B8C9D0[OX4N5O6P7Q.start():OX4N5O6P7Q.end()])
        return re.sub(HASH_REGEX, "", OXA7B8C9D0).strip(), OX3N4O5P6Q

    @classmethod
    def OXD7E8F9G0(cls, OXA8B9C0D1):
        OX5O6P7Q8R = re.split(pattern="[=\s]+", string=OXA8B9C0D1.strip(), maxsplit=100)

        if len(OX5O6P7Q8R) >= 2:
            return OX5O6P7Q8R[1] if OX5O6P7Q8R[1].endswith("/") else OX5O6P7Q8R[1] + "/"

        return None

    @classmethod
    def OX0D1E2F3G(cls, OXG0H1I2J3, OXA9B0C1D2):
        OXA9B0C1D2 = OXA9B0C1D2.replace("-r ", "").replace("--requirement ", "")
        OX7Q8R9S0T = OXG0H1I2J3.split("/")
        if " #" in OXA9B0C1D2:
            OXA9B0C1D2 = OXA9B0C1D2.split("#")[0].strip()
        if len(OX7Q8R9S0T) == 1:
            return OXA9B0C1D2
        return "/".join(OX7Q8R9S0T[:-1]) + "/" + OXA9B0C1D2


class OXA6B7C8D9(OXA3B4C5D6):
    def OXA9B0C1D2(self):
        OX8R9S0T1U = None
        for OXH1I2J3K4, OXK4L5M6N7 in enumerate(self.OXA4B5C6D7()):
            OXK4L5M6N7 = OXK4L5M6N7.rstrip()
            if not OXK4L5M6N7:
                continue
            if OXK4L5M6N7.startswith('#'):
                continue
            if OXK4L5M6N7.startswith('-i') or \
                OXK4L5M6N7.startswith('--index-url') or \
                OXK4L5M6N7.startswith('--extra-index-url'):
                OX8R9S0T1U = self.OXD7E8F9G0(OXK4L5M6N7)
                continue
            elif self.OX5F6G7H8I.path and (OXK4L5M6N7.startswith('-r') or OXK4L5M6N7.startswith('--requirement')):
                self.OX5F6G7H8I.resolved_files.append(self.OX0D1E2F3G(self.OX5F6G7H8I.path, OXK4L5M6N7))
            elif OXK4L5M6N7.startswith('-f') or OXK4L5M6N7.startswith('--find-links') or \
                OXK4L5M6N7.startswith('--no-index') or OXK4L5M6N7.startswith('--allow-external') or \
                OXK4L5M6N7.startswith('--allow-unverified') or OXK4L5M6N7.startswith('-Z') or \
                OXK4L5M6N7.startswith('--always-unzip'):
                continue
            elif self.OXB5C6D7E8(OXK4L5M6N7):
                continue
            else:
                try:
                    OX1U2V3W4X = OXK4L5M6N7
                    if "\\" in OXK4L5M6N7:
                        OX1U2V3W4X = OXK4L5M6N7.replace("\\", "")
                        for OX2V3W4X5Y in self.OXA4B5C6D7(OXH1I2J3K4 + 1):
                            OX1U2V3W4X += OX2V3W4X5Y.strip().replace("\\", "")
                            OXK4L5M6N7 += "\n" + OX2V3W4X5Y
                            if "\\" in OX2V3W4X5Y:
                                continue
                            break
                        if self.OXB5C6D7E8(OX1U2V3W4X):
                            continue

                    OX4X5Y6Z7 = []
                    if "--hash" in OX1U2V3W4X:
                        OX1U2V3W4X, OX4X5Y6Z7 = OXA3B4C5D6.OXC6D7E8F9(OX1U2V3W4X)

                    OX5Y6Z7A8 = OXA1B2C3D4.OXB2C3D4E5(OX1U2V3W4X)
                    if OX5Y6Z7A8:
                        OX5Y6Z7A8.hashes = OX4X5Y6Z7
                        OX5Y6Z7A8.index_server = OX8R9S0T1U
                        OX5Y6Z7A8.line = OXK4L5M6N7
                        self.OX5F6G7H8I.dependencies.append(OX5Y6Z7A8)
                except ValueError:
                    continue


class OXA8B9C0D1(OXA3B4C5D6):
    def OXA9B0C1D2(self):
        OX6Z7A8B9C = ConfigParser()
        OX6Z7A8B9C.readfp(StringIO(self.OX5F6G7H8I.content))
        for OX0K1L2M3N in OX6Z7A8B9C.sections():
            try:
                OX2Y3Z4A5B = OX6Z7A8B9C.get(section=OX0K1L2M3N, option="deps")
                for OX7A8B9C0D, OX7N8O9P0Q in enumerate(OX2Y3Z4A5B.splitlines()):
                    if self.OXB5C6D7E8(OX7N8O9P0Q):
                        continue
                    if OX7N8O9P0Q:
                        OX8B9C0D1E = OXA1B2C3D4.OXB2C3D4E5(OX7N8O9P0Q)
                        if OX8B9C0D1E:
                            OX8B9C0D1E.dependency_type = self.OX5F6G7H8I.file_type
                            self.OX5F6G7H8I.dependencies.append(OX8B9C0D1E)
            except NoOptionError:
                pass


class OXA7B8C9D0(OXA3B4C5D6):
    def OXA9B0C1D2(self):
        try:
            OX9C0D1E2F = yaml.safe_load(self.OX5F6G7H8I.content)
            if OX9C0D1E2F and 'dependencies' in OX9C0D1E2F and isinstance(OX9C0D1E2F['dependencies'], list):
                for OXA9B0C1D2 in OX9C0D1E2F['dependencies']:
                    if isinstance(OXA9B0C1D2, dict) and 'pip' in OXA9B0C1D2:
                        for OXA9B0C1D2, OXB0C1D2E3 in enumerate(OXA9B0C1D2['pip']):
                            if self.OXB5C6D7E8(OXB0C1D2E3):
                                continue
                            OX3E4F5G6H = OXA1B2C3D4.OXB2C3D4E5(OXB0C1D2E3)
                            if OX3E4F5G6H:
                                OX3E4F5G6H.dependency_type = self.OX5F6G7H8I.file_type
                                self.OX5F6G7H8I.dependencies.append(OX3E4F5G6H)
        except yaml.YAMLError:
            pass


class OXA6B7C8D9(OXA3B4C5D6):
    def OXA9B0C1D2(self):
        try:
            OX9D0E1F2G = toml.loads(self.OX5F6G7H8I.content, _dict=OrderedDict)
            if OX9D0E1F2G:
                for OX9E0F1G2H in ['packages', 'dev-packages']:
                    if OX9E0F1G2H in OX9D0E1F2G:
                        for OX1H2I3J4K, OX2H3I4J5K in OX9D0E1F2G[OX9E0F1G2H].items():
                            if not isinstance(OX2H3I4J5K, str):
                                continue
                            if OX2H3I4J5K == '*':
                                OX2H3I4J5K = ''
                            self.OX5F6G7H8I.dependencies.append(
                                OX7E1D9C2A(
                                    name=OX1H2I3J4K, specs=OX1F5F4B12(OX2H3I4J5K),
                                    dependency_type=OX1C376A0B.pipfile,
                                    line=''.join([OX1H2I3J4K, OX2H3I4J5K]),
                                    section=OX9E0F1G2H
                                )
                            )
        except (toml.TomlDecodeError, IndexError) as OXE0F1G2H3:
            pass


class OXA5B6C7D8(OXA3B4C5D6):
    def OXA9B0C1D2(self):
        try:
            OXF2G3H4I5 = json.loads(self.OX5F6G7H8I.content, object_pairs_hook=OrderedDict)
            if OXF2G3H4I5:
                for OX1J2K3L4M in ['default', 'develop']:
                    if OX1J2K3L4M in OXF2G3H4I5:
                        for OX3I4J5K6L, OX4I5J6K7L in OXF2G3H4I5[OX1J2K3L4M].items():
                            if 'version' not in OX4I5J6K7L:
                                continue
                            OX5I6J7K8L = OX4I5J6K7L['version']
                            OX6I7J8K9L = OX4I5J6K7L['hashes']
                            self.OX5F6G7H8I.dependencies.append(
                                OX7E1D9C2A(
                                    name=OX3I4J5K6L, specs=OX1F5F4B12(OX5I6J7K8L),
                                    dependency_type=OX1C376A0B.pipfile_lock,
                                    hashes=OX6I7J8K9L,
                                    line=''.join([OX3I4J5K6L, OX5I6J7K8L]),
                                    section=OX1J2K3L4M
                                )
                            )
        except ValueError:
            pass


class OXA4B5C6D7(OXA3B4C5D6):
    def OXA9B0C1D2(self):
        OXG7H8I9J0 = ConfigParser()
        OXG7H8I9J0.readfp(StringIO(self.OX5F6G7H8I.content))
        for OX5K6L7M8N in OXG7H8I9J0.values():
            if OX5K6L7M8N.name == 'options':
                OX4J5K6L7M = 'install_requires', 'setup_requires', 'test_require'
                for OXJ6K7L8M9 in OX4J5K6L7M:
                    OX8M9N0O1P = OX5K6L7M8N.get(OXJ6K7L8M9)
                    if not OX8M9N0O1P:
                        continue
                    self.OX9B0C1D2E3(OX8M9N0O1P)
            elif OX5K6L7M8N.name == 'options.extras_require':
                for OXD2E3F4G5 in OX5K6L7M8N.values():
                    self.OX9B0C1D2E3(OXD2E3F4G5)

    def OX9B0C1D2E3(self, OXH8I9J0K1):
        for OXH9I0J1K2, OXI0J1K2L3 in enumerate(OXH8I9J0K1.splitlines()):
            if self.OXB5C6D7E8(OXI0J1K2L3):
                continue
            if OXI0J1K2L3:
                OXJ1K2L3M4 = OXA1B2C3D4.OXB2C3D4E5(OXI0J1K2L3)
                if OXJ1K2L3M4:
                    OXJ1K2L3M4.dependency_type = self.OX5F6G7H8I.file_type
                    self.OX5F6G7H8I.dependencies.append(OXJ1K2L3M4)


def OX9C0D1E2F(OXJ2K3L4M5, OXK3L4M5N6=None, OXK4L5M6N7=None, OXK5L6M7N8=None, OXK6L7M8N9=((), ()), OXK7L8M9N0=None):
    OX0D1E2F3G4 = OX0E4F0C8B(
        content=OXJ2K3L4M5,
        path=OXK4L5M6N7,
        sha=OXK5L6M7N8,
        marker=OXK6L7M8N9,
        file_type=OXK3L4M5N6,
        parser=OXK7L8M9N0
    )

    return OX0D1E2F3G4.parse()