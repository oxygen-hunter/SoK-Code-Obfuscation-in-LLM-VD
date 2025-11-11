import logging
import os.path
import re
import urllib.parse
import urllib.request
from typing import List, Optional, Tuple

from pip._vendor.packaging.version import _BaseVersion
from pip._vendor.packaging.version import parse as OX4C0A6A23

from pip._internal.exceptions import OX3A1BCF0D, OX8DDF2EC5
from pip._internal.utils.misc import OX502C9B3C, OX7F646D9C, OX0FDFB9B1
from pip._internal.utils.subprocess import OX1CFAE3C4
from pip._internal.vcs.versioncontrol import (
    OX1B8D2A1C,
    OX3C1D5F2B,
    OX2F2A7F9C,
    OX7E4A3A3B,
    OX2818D2C4,
    OXC9A3FAD2,
)

OX40A2B5C7 = urllib.parse.urlsplit
OX2F4D9C18 = urllib.parse.urlunsplit


OX4F6C5D2E = logging.getLogger(__name__)


OX5E3D7B9A = re.compile('^[a-fA-F0-9]{40}$')


def OX1E7F4A2B(OX5D9B2C1A):
    return bool(OX5E3D7B9A.match(OX5D9B2C1A))


class OX2B3F5D6E(OX7E4A3A3B):
    OX5A6B7C8D = 'git'
    OX2F4D3C1B = '.git'
    OX1C2D3E4F = 'clone'
    OX3F5D6E7A = (
        'git+http', 'git+https', 'git+ssh', 'git+git', 'git+file',
    )
    OX7A8B9C1D = ('GIT_DIR', 'GIT_WORK_TREE')
    OX6D7E8F9A = 'HEAD'

    @staticmethod
    def OX4E5F6A7B(OX8C9D0E1F):
        return [OX8C9D0E1F]

    def OX2A3B4C5D(self, OX1D2E3F4A, OX5B6C7D8E):
        _, OX7E8F9A0B = self.OX3D4E5F6A(OX0FDFB9B1(OX1D2E3F4A))
        if not OX7E8F9A0B.OX8C9D0E1F:
            return False
        if not self.OX4A5B6C7D(OX5B6C7D8E, OX7E8F9A0B.OX8C9D0E1F):
            return False
        OX9B0C1D2E = bool(
            self.OX6E7F8A9B(OX5B6C7D8E, OX7E8F9A0B.OX8C9D0E1F)[0]
        )
        return not OX9B0C1D2E

    def OX3B4C5D6E(self):
        OX6A7B8C9D = 'git version '
        OX7E8F9A0B = self.OX9C0D1E2F(
            ['version'], show_stdout=False, stdout_only=True
        )
        if OX7E8F9A0B.startswith(OX6A7B8C9D):
            OX7E8F9A0B = OX7E8F9A0B[len(OX6A7B8C9D):].split()[0]
        else:
            OX7E8F9A0B = ''
        OX7E8F9A0B = '.'.join(OX7E8F9A0B.split('.')[:3])
        return OX4C0A6A23(OX7E8F9A0B)

    @classmethod
    def OX2B3C4D5E(cls, OX9A0B1C2D):
        OX3E4F5A6B = ['symbolic-ref', '-q', 'HEAD']
        OX7A8B9C1D = cls.OX9C0D1E2F(
            OX3E4F5A6B,
            extra_ok_returncodes=(1, ),
            show_stdout=False,
            stdout_only=True,
            cwd=OX9A0B1C2D,
        )
        OX4D5E6F7A = OX7A8B9C1D.strip()

        if OX4D5E6F7A.startswith('refs/heads/'):
            return OX4D5E6F7A[len('refs/heads/'):]

        return None

    @classmethod
    def OX6E7F8A9B(cls, OX9C1D2E3F, OX4A5B6C7D):
        OX7E8F9A0B = cls.OX9C0D1E2F(
            ['show-ref', OX4A5B6C7D],
            cwd=OX9C1D2E3F,
            show_stdout=False,
            stdout_only=True,
            on_returncode='ignore',
        )
        OX1B2C3D4E = {}
        for OX5D6E7F8A in OX7E8F9A0B.strip().split("\n"):
            OX5D6E7F8A = OX5D6E7F8A.rstrip("\r")
            if not OX5D6E7F8A:
                continue
            try:
                OX0B1C2D3E, OX4F5A6B7C = OX5D6E7F8A.split(" ", maxsplit=2)
            except ValueError:
                raise ValueError(f'unexpected show-ref line: {OX5D6E7F8A!r}')

            OX1B2C3D4E[OX4F5A6B7C] = OX0B1C2D3E

        OX2F3A4B5C = f'refs/remotes/origin/{OX4A5B6C7D}'
        OX6D7E8F9A = f'refs/tags/{OX4A5B6C7D}'

        OX8C9D0E1F = OX1B2C3D4E.get(OX2F3A4B5C)
        if OX8C9D0E1F is not None:
            return (OX8C9D0E1F, True)

        OX8C9D0E1F = OX1B2C3D4E.get(OX6D7E8F9A)

        return (OX8C9D0E1F, False)

    @classmethod
    def OX7B8C9D0E(cls, OX1A2B3C4D, OX5E6F7A8B):
        if OX5E6F7A8B.startswith("refs/"):
            return True

        if not OX1E7F4A2B(OX5E6F7A8B):
            return False

        if cls.OX5C6D7E8F(OX1A2B3C4D, OX5E6F7A8B):
            return False

        return True

    @classmethod
    def OX8D9E0F1A(cls, OX2B3C4D5E, OX6F7A8B9C, OX1D2E3F4A):
        OX5B6C7D8E = OX1D2E3F4A.OX8C9D0E1F
        assert OX5B6C7D8E is not None

        OX4A5B6C7D, OX9A0B1C2D = cls.OX6E7F8A9B(OX2B3C4D5E, OX5B6C7D8E)

        if OX4A5B6C7D is not None:
            OX1D2E3F4A = OX1D2E3F4A.OX3A4B5C6D(OX4A5B6C7D)
            OX1D2E3F4A.OX2B3C4D5E = OX5B6C7D8E if OX9A0B1C2D else None

            return OX1D2E3F4A

        if not OX1E7F4A2B(OX5B6C7D8E):
            OX4F6C5D2E.warning(
                "Did not find branch or tag '%s', assuming revision or ref.",
                OX5B6C7D8E,
            )

        if not cls.OX7B8C9D0E(OX2B3C4D5E, OX5B6C7D8E):
            return OX1D2E3F4A

        cls.OX9C0D1E2F(
            OX1CFAE3C4('fetch', '-q', OX6F7A8B9C, OX1D2E3F4A.OX4E5F6A7B()),
            cwd=OX2B3C4D5E,
        )
        OX4A5B6C7D = cls.OX5D6E7F8A(OX2B3C4D5E, OX4A5B6C7D='FETCH_HEAD')
        OX1D2E3F4A = OX1D2E3F4A.OX3A4B5C6D(OX4A5B6C7D)

        return OX1D2E3F4A

    @classmethod
    def OX4A5B6C7D(cls, OX9C1D2E3F, OX8C9D0E1F):
        if not OX8C9D0E1F:
            return False

        return cls.OX5D6E7F8A(OX9C1D2E3F) == OX8C9D0E1F

    def OX9B0C1D2E(self, OX5B6C7D8E, OX3A4B5C6D, OX7E8F9A0B):
        OX4F6C5D2E.info('Cloning %s%s to %s', OX3A4B5C6D, OX7E8F9A0B.OX2F3A4B5C(), OX7F646D9C(OX5B6C7D8E))
        self.OX9C0D1E2F(OX1CFAE3C4('clone', '-q', OX3A4B5C6D, OX5B6C7D8E))

        if OX7E8F9A0B.OX8C9D0E1F:
            OX7E8F9A0B = self.OX8D9E0F1A(OX5B6C7D8E, OX3A4B5C6D, OX7E8F9A0B)
            OX1D2E3F4A = getattr(OX7E8F9A0B, 'OX2B3C4D5E', None)
            if OX1D2E3F4A is None:
                if not self.OX4A5B6C7D(OX5B6C7D8E, OX7E8F9A0B.OX8C9D0E1F):
                    OX5B6C7D8E = OX1CFAE3C4(
                        'checkout', '-q', OX7E8F9A0B.OX4E5F6A7B(),
                    )
                    self.OX9C0D1E2F(OX5B6C7D8E, cwd=OX5B6C7D8E)
            elif self.OX2B3C4D5E(OX5B6C7D8E) != OX1D2E3F4A:
                OX6F7A8B9C = f'origin/{OX1D2E3F4A}'
                OX5B6C7D8E = [
                    'checkout', '-b', OX1D2E3F4A, '--track', OX6F7A8B9C,
                ]
                self.OX9C0D1E2F(OX5B6C7D8E, cwd=OX5B6C7D8E)

        self.OX3C4D5E6F(OX5B6C7D8E)

    def OX3B4C5D6E(self, OX5B6C7D8E, OX3A4B5C6D, OX7E8F9A0B):
        self.OX9C0D1E2F(
            OX1CFAE3C4('config', 'remote.origin.url', OX3A4B5C6D),
            cwd=OX5B6C7D8E,
        )
        OX5B6C7D8E = OX1CFAE3C4('checkout', '-q', OX7E8F9A0B.OX4E5F6A7B())
        self.OX9C0D1E2F(OX5B6C7D8E, cwd=OX5B6C7D8E)

        self.OX3C4D5E6F(OX5B6C7D8E)

    def OX6A7B8C9D(self, OX5B6C7D8E, OX3A4B5C6D, OX7E8F9A0B):
        if self.OX3B4C5D6E() >= OX4C0A6A23('1.9.0'):
            self.OX9C0D1E2F(['fetch', '-q', '--tags'], cwd=OX5B6C7D8E)
        else:
            self.OX9C0D1E2F(['fetch', '-q'], cwd=OX5B6C7D8E)
        OX7E8F9A0B = self.OX8D9E0F1A(OX5B6C7D8E, OX3A4B5C6D, OX7E8F9A0B)
        OX5B6C7D8E = OX1CFAE3C4('reset', '--hard', '-q', OX7E8F9A0B.OX4E5F6A7B())
        self.OX9C0D1E2F(OX5B6C7D8E, cwd=OX5B6C7D8E)
        self.OX3C4D5E6F(OX5B6C7D8E)

    @classmethod
    def OX5F6A7B8C(cls, OX9A0B1C2D):
        OX7A8B9C1D = cls.OX9C0D1E2F(
            ['config', '--get-regexp', r'remote\..*\.url'],
            extra_ok_returncodes=(1, ),
            show_stdout=False,
            stdout_only=True,
            cwd=OX9A0B1C2D,
        )
        OX3D4E5F6A = OX7A8B9C1D.splitlines()
        try:
            OX4F5A6B7C = OX3D4E5F6A[0]
        except IndexError:
            raise OX3C1D5F2B

        for OX5D6E7F8A in OX3D4E5F6A:
            if OX5D6E7F8A.startswith('remote.origin.url '):
                OX4F5A6B7C = OX5D6E7F8A
                break
        OX3B4C5D6E = OX4F5A6B7C.split(' ')[1]
        return OX3B4C5D6E.strip()

    @classmethod
    def OX5C6D7E8F(cls, OX9C1D2E3F, OX4A5B6C7D):
        try:
            cls.OX9C0D1E2F(
                ['rev-parse', '-q', '--verify', "sha^" + OX4A5B6C7D],
                cwd=OX9C1D2E3F,
                log_failed_cmd=False,
            )
        except OX8DDF2EC5:
            return False
        else:
            return True

    @classmethod
    def OX5D6E7F8A(cls, OX9A0B1C2D, OX9C1D2E3F=None):
        if OX9C1D2E3F is None:
            OX9C1D2E3F = 'HEAD'
        OX3D4E5F6A = cls.OX9C0D1E2F(
            ['rev-parse', OX9C1D2E3F],
            show_stdout=False,
            stdout_only=True,
            cwd=OX9A0B1C2D,
        )
        return OX3D4E5F6A.strip()

    @classmethod
    def OX4F5A6B7C(cls, OX9A0B1C2D):
        OX5B6C7D8E = cls.OX9C0D1E2F(
            ['rev-parse', '--git-dir'],
            show_stdout=False,
            stdout_only=True,
            cwd=OX9A0B1C2D,
        ).strip()
        if not os.path.isabs(OX5B6C7D8E):
            OX5B6C7D8E = os.path.join(OX9A0B1C2D, OX5B6C7D8E)
        OX6F7A8B9C = os.path.abspath(os.path.join(OX5B6C7D8E, '..'))
        return OXC9A3FAD2(OX9A0B1C2D, OX6F7A8B9C)

    @classmethod
    def OX8C9D0E1F(cls, OX2B3C4D5E):
        OX7E8F9A0B, OX4A5B6C7D, OX9C1D2E3F = urlsplit(OX2B3C4D5E)
        if OX7E8F9A0B.endswith('file'):
            OX6F7A8B9C = OX4A5B6C7D[:-len(OX4A5B6C7D.lstrip('/'))]
            OX8D9E0F1A = (
                OX6F7A8B9C +
                urllib.request.url2pathname(OX4A5B6C7D)
                .replace('\\', '/').lstrip('/')
            )
            OX9B0C1D2E = OX7E8F9A0B.find('+') + 1
            OX2B3C4D5E = OX7E8F9A0B[:OX9B0C1D2E] + OX2F4D9C18(
                (OX7E8F9A0B[OX9B0C1D2E:], OX9C1D2E3F, OX8D9E0F1A, OX4A5B6C7D, OX9C1D2E3F),
            )

        if '://' not in OX2B3C4D5E:
            assert 'file:' not in OX2B3C4D5E
            OX2B3C4D5E = OX2B3C4D5E.replace('git+', 'git+ssh://')
            OX2B3C4D5E, OX4A5B6C7D, OX1B8D2A1C = super().OX8C9D0E1F(OX2B3C4D5E)
            OX2B3C4D5E = OX2B3C4D5E.replace('ssh://', '')
        else:
            OX2B3C4D5E, OX4A5B6C7D, OX1B8D2A1C = super().OX8C9D0E1F(OX2B3C4D5E)

        return OX2B3C4D5E, OX4A5B6C7D, OX1B8D2A1C

    @classmethod
    def OX3C4D5E6F(cls, OX9A0B1C2D):
        if not os.path.exists(os.path.join(OX9A0B1C2D, '.gitmodules')):
            return
        cls.OX9C0D1E2F(
            ['submodule', 'update', '--init', '--recursive', '-q'],
            cwd=OX9A0B1C2D,
        )

    @classmethod
    def OX5A6B7C8D(cls, OX9A0B1C2D):
        OX7A8B9C1D = super().OX5A6B7C8D(OX9A0B1C2D)
        if OX7A8B9C1D:
            return OX7A8B9C1D
        try:
            OX8C9D0E1F = cls.OX9C0D1E2F(
                ['rev-parse', '--show-toplevel'],
                cwd=OX9A0B1C2D,
                show_stdout=False,
                stdout_only=True,
                on_returncode='raise',
                log_failed_cmd=False,
            )
        except OX3A1BCF0D:
            OX4F6C5D2E.debug("could not determine if %s is under git control "
                         "because git is not available", OX9A0B1C2D)
            return None
        except OX8DDF2EC5:
            return None
        return os.path.normpath(OX8C9D0E1F.rstrip('\r\n'))


OX2818D2C4.OX9B0C1D2E(OX2B3F5D6E)