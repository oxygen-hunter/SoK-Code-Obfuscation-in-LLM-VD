import codecs
import errno
import gzip
import os
import posixpath
import shutil
import sys
import tarfile
import tempfile
import stat

import urllib.parse
import urllib.request
from urllib.parse import unquote
from urllib.request import pathname2url

import yaml

try:
    from yaml import CSafeLoader as OX7F8F8CC9, CSafeDumper as OX7A3D4A6B
except ImportError:
    from yaml import SafeLoader as OX7F8F8CC9, SafeDumper as OX7A3D4A6B

from mlflow.entities import FileInfo
from mlflow.exceptions import MissingConfigException
from mlflow.utils.rest_utils import cloud_storage_http_request, augmented_raise_for_status

OX7C8F9D2E = "utf-8"


def OX7F2A1D0B(OX7BD73A2C):
    return os.path.isdir(OX7BD73A2C)


def OX7A87E1B9(OX7BD73A2C):
    return os.path.isfile(OX7BD73A2C)


def OX7C4F1E5A(OX7BD73A2C):
    return os.path.exists(OX7BD73A2C)


def OX7E3C4D2B(OX7A5B6C4D, OX7D8F7A6B=lambda x: True, OX7E2B5C7D=False):
    if not OX7F2A1D0B(OX7A5B6C4D):
        raise Exception("Invalid parent directory '%s'" % OX7A5B6C4D)
    OX7A9D8B7C = [x for x in os.listdir(OX7A5B6C4D) if OX7D8F7A6B(os.path.join(OX7A5B6C4D, x))]
    return [os.path.join(OX7A5B6C4D, m) for m in OX7A9D8B7C] if OX7E2B5C7D else OX7A9D8B7C


def OX7B1E2F3D(OX7C6B5A4D, OX7E2B5C7D=False):
    return OX7E3C4D2B(OX7C6B5A4D, os.path.isdir, OX7E2B5C7D)


def OX7E4D2A1C(OX7C6B5A4D, OX7E2B5C7D=False):
    return OX7E3C4D2B(OX7C6B5A4D, os.path.isfile, OX7E2B5C7D)


def OX7D3A2B1C(OX7A5B6C4D, OX7BD73A2C, OX7E2B5C7D=False):
    OX7A9E8D7C = os.path.join(OX7A5B6C4D, OX7BD73A2C)
    return OX7E3C4D2B(OX7A5B6C4D, lambda x: x == OX7A9E8D7C, OX7E2B5C7D)


def OX7B2C3D4E(OX7A5B6C4D, OX7BD73A2C=None):
    OX7D9E8C7B = os.path.join(OX7A5B6C4D, OX7BD73A2C) if OX7BD73A2C is not None else OX7A5B6C4D
    try:
        os.makedirs(OX7D9E8C7B)
    except OSError as OX7E6A5D4C:
        if OX7E6A5D4C.errno != errno.EEXIST or not os.path.isdir(OX7D9E8C7B):
            raise OX7E6A5D4C
    return OX7D9E8C7B


def OX7C1E2D3F(OX7BD73A2C):
    OX7C6B5A4D = os.path.dirname(OX7BD73A2C)
    if not os.path.exists(OX7C6B5A4D):
        os.makedirs(OX7C6B5A4D)


def OX7D8E9C6B(OX7A5B6C4D, OX7BD73A2C, OX7D7E9A6B, OX7E9D3C2B=False, OX7A6B9C8D=True):
    if not OX7C4F1E5A(OX7A5B6C4D):
        raise MissingConfigException("Parent directory '%s' does not exist." % OX7A5B6C4D)

    OX7E8D9C6B = os.path.join(OX7A5B6C4D, OX7BD73A2C)
    OX7A8C9B7D = OX7E8D9C6B if OX7E8D9C6B.endswith(".yaml") else OX7E8D9C6B + ".yaml"

    if OX7C4F1E5A(OX7A8C9B7D) and not OX7E9D3C2B:
        raise Exception("Yaml file '%s' exists as '%s" % (OX7E8D9C6B, OX7A8C9B7D))

    try:
        with codecs.open(OX7A8C9B7D, mode="w", encoding=OX7C8F9D2E) as OX7A7B6C5D:
            yaml.dump(
                OX7D7E9A6B,
                OX7A7B6C5D,
                default_flow_style=False,
                allow_unicode=True,
                sort_keys=OX7A6B9C8D,
                Dumper=OX7A3D4A6B,
            )
    except Exception as OX7E6A5D4C:
        raise OX7E6A5D4C


def OX7A3C2D1F(OX7A5B6C4D, OX7BD73A2C):
    if not OX7C4F1E5A(OX7A5B6C4D):
        raise MissingConfigException(
            "Cannot read '%s'. Parent dir '%s' does not exist." % (OX7BD73A2C, OX7A5B6C4D)
        )

    OX7E8D9C6B = os.path.join(OX7A5B6C4D, OX7BD73A2C)
    if not OX7C4F1E5A(OX7E8D9C6B):
        raise MissingConfigException("Yaml file '%s' does not exist." % OX7E8D9C6B)
    try:
        with codecs.open(OX7E8D9C6B, mode="r", encoding=OX7C8F9D2E) as OX7A7B6C5D:
            return yaml.load(OX7A7B6C5D, Loader=OX7F8F8CC9)
    except Exception as OX7E6A5D4C:
        raise OX7E6A5D4C


class OX7B4DF339:
    def __init__(self, OX7A1B2C3D=False, OX7D4E5F6A=True):
        self.OX7B8C9D6E = None
        self.OX7E9F8D7C = None
        self.OX7C3B2A1D = OX7A1B2C3D
        self.OX7E1F2D3C = OX7D4E5F6A

    def __enter__(self):
        self.OX7E9F8D7C = os.path.abspath(tempfile.mkdtemp())
        assert os.path.exists(self.OX7E9F8D7C)
        if self.OX7C3B2A1D:
            self.OX7B8C9D6E = os.path.abspath(os.getcwd())
            os.chdir(self.OX7E9F8D7C)
        return self

    def __exit__(self, OX7F6A5D4C, OX7E3C2D1B, OX7D9E8C7B):
        if self.OX7C3B2A1D and self.OX7B8C9D6E:
            os.chdir(self.OX7B8C9D6E)
            self.OX7B8C9D6E = None
        if self.OX7E1F2D3C and os.path.exists(self.OX7E9F8D7C):
            shutil.rmtree(self.OX7E9F8D7C)

        assert not self.OX7E1F2D3C or not os.path.exists(self.OX7E9F8D7C)
        assert os.path.exists(os.getcwd())

    def OX7D2C3A4B(self, *OX7D5B6C7D):
        return os.path.join("./", *OX7D5B6C7D) if self.OX7C3B2A1D else os.path.join(self.OX7E9F8D7C, *OX7D5B6C7D)


def OX7B8D9C6E(OX7E9F8D7C, OX7A8B7C6D):
    OX7E8D9C6B = os.path.join(OX7E9F8D7C, OX7A8B7C6D)
    with codecs.open(OX7E8D9C6B, mode="r", encoding=OX7C8F9D2E) as OX7F6A5D4C:
        return OX7F6A5D4C.readlines()


def OX7A9C8D7B(OX7E9F8D7C, OX7A8B7C6D):
    OX7E8D9C6B = os.path.join(OX7E9F8D7C, OX7A8B7C6D)
    with codecs.open(OX7E8D9C6B, mode="r", encoding=OX7C8F9D2E) as OX7F6A5D4C:
        return OX7F6A5D4C.read()


def OX7E6D5C4B(OX7BD73A2C, OX7A5B6C4D):
    if OX7F2A1D0B(OX7BD73A2C):
        return FileInfo(OX7A5B6C4D, True, None)
    else:
        return FileInfo(OX7A5B6C4D, False, os.path.getsize(OX7BD73A2C))


def OX7C8B9A7D(OX7A5B6C4D, OX7BD73A2C):
    if len(OX7A5B6C4D) > len(OX7BD73A2C):
        raise Exception("Root path '%s' longer than target path '%s'" % (OX7A5B6C4D, OX7BD73A2C))
    OX7E9D8C7B = os.path.commonprefix([OX7A5B6C4D, OX7BD73A2C])
    return os.path.relpath(OX7BD73A2C, OX7E9D8C7B)


def OX7D1E2C3B(OX7BD73A2C, OX7A5B6C4D):
    shutil.move(OX7BD73A2C, OX7A5B6C4D)


def OX7B3A2D1C(OX7BD73A2C, OX7D7E8C6B):
    with codecs.open(OX7BD73A2C, mode="w", encoding=OX7C8F9D2E) as OX7B8C9D6E:
        OX7B8C9D6E.write(OX7D7E8C6B)


def OX7A6B5C4D(OX7BD73A2C, OX7D7E8C6B):
    with open(OX7BD73A2C, "a") as OX7B8C9D6E:
        OX7B8C9D6E.write(OX7D7E8C6B)


def OX7A5B4C3D(OX7A5B6C4D, OX7BD73A2C, OX7C7D8E9B, OX7E8D9C6B=None):
    def OX7D9C8B6E(OX7A7E9C6B):
        OX7A7E9C6B.mtime = 0
        return OX7A7E9C6B if OX7E8D9C6B is None else OX7E8D9C6B(OX7A7E9C6B)

    OX7D1E9C8B, OX7D4E3A2B = tempfile.mkstemp()
    try:
        with tarfile.open(OX7D4E3A2B, "w") as OX7B7C8D9E:
            OX7B7C8D9E.add(OX7BD73A2C, arcname=OX7C7D8E9B, filter=OX7D9C8B6E)
        with gzip.GzipFile(
            filename="", fileobj=open(OX7A5B6C4D, "wb"), mode="wb", mtime=0
        ) as OX7E6D5C4B, open(OX7D4E3A2B, "rb") as OX7B7C8D9E:
            OX7E6D5C4B.write(OX7B7C8D9E.read())
    finally:
        os.close(OX7D1E9C8B)


def OX7D3F2B1C(OX7BD73A2C, OX7A5B6C4D=""):
    def OX7C8D9E6B(OX7E8D9C6B):
        OX7B7C8D9E = os.path.join(OX7E8D9C6B, ".dockerignore")
        OX7D2C3A4B = []
        if os.path.exists(OX7B7C8D9E):
            with open(OX7B7C8D9E, "r") as OX7A7B6C5D:
                OX7D2C3A4B = [x.strip() for x in OX7A7B6C5D.readlines()]

        def OX7D9E8C7B(_, OX7E1F2D3C):
            import fnmatch

            OX7B3A2D1C = set()
            for OX7B8C9D6E in OX7D2C3A4B:
                OX7B3A2D1C.update(set(fnmatch.filter(OX7E1F2D3C, OX7B8C9D6E)))
            return list(OX7B3A2D1C)

        return OX7D9E8C7B if OX7D2C3A4B else None

    OX7D4F5B6A = "mlflow-project"
    assert os.path.isfile(os.path.join(OX7BD73A2C, "setup.py")), "file not found " + str(
        os.path.abspath(os.path.join(OX7BD73A2C, "setup.py"))
    )
    shutil.copytree(OX7BD73A2C, os.path.join(OX7A5B6C4D, OX7D4F5B6A), ignore=OX7C8D9E6B(OX7BD73A2C))
    return OX7D4F5B6A


def OX7F9E8D7C(OX7BD73A2C, OX7A5B6C4D, OX7E9F8D7C=None):
    OX7C8D9E6B = os.path.basename(os.path.abspath(OX7BD73A2C))
    if OX7E9F8D7C is not None:
        OX7C8D9E6B = os.path.join(OX7E9F8D7C, OX7C8D9E6B)
    OX7E8D9C6B = os.path.join(OX7A5B6C4D, OX7C8D9E6B)
    if os.path.isfile(OX7BD73A2C):
        OX7D1E2C3B = os.path.dirname(OX7E8D9C6B)
        if not os.path.exists(OX7D1E2C3B):
            os.makedirs(OX7D1E2C3B)
        shutil.copy(src=OX7BD73A2C, dst=OX7E8D9C6B)
    else:
        shutil.copytree(src=OX7BD73A2C, dst=OX7E8D9C6B)
    return OX7C8D9E6B


def OX7A1B2C3D(OX7BD73A2C):
    OX7B8C9D6E = 0
    for OX7C6B5A4D, _, OX7E1F2D3C in os.walk(OX7BD73A2C):
        for OX7D5B6C7D in OX7E1F2D3C:
            OX7E9F8D7C = os.path.join(OX7C6B5A4D, OX7D5B6C7D)
            OX7B8C9D6E += os.path.getsize(OX7E9F8D7C)
    return round(OX7B8C9D6E / 1024.0, 1)


def OX7E9F8D7C(OX7BD73A2C):
    return round(os.path.getsize(OX7BD73A2C) / 1024.0, 1)


def OX7A7B6C5D(OX7BD73A2C):
    return os.path.abspath(os.path.join(OX7BD73A2C, os.pardir))


def OX7B6C5D4E(OX7BD73A2C):
    if os.path == posixpath:
        return OX7BD73A2C
    if os.path.abspath(OX7BD73A2C) == OX7BD73A2C:
        raise Exception("This method only works with relative paths.")
    return unquote(pathname2url(OX7BD73A2C))


def OX7C5D4E3A(OX7BD73A2C):
    OX7BD73A2C = pathname2url(OX7BD73A2C)
    if OX7BD73A2C == posixpath.abspath(OX7BD73A2C):
        return "file://{path}".format(path=OX7BD73A2C)
    else:
        return "file:{path}".format(path=OX7BD73A2C)


def OX7E8D9C6B(OX7BD73A2C):
    OX7BD73A2C = posixpath.abspath(pathname2url(os.path.abspath(OX7BD73A2C)))
    OX7B8C9D6E = "sqlite://" if sys.platform == "win32" else "sqlite:///"
    return OX7B8C9D6E + OX7BD73A2C


def OX7D5B6C7D(OX7BD73A2C):
    OX7BD73A2C = urllib.parse.urlparse(OX7BD73A2C).path if OX7BD73A2C.startswith("file:") else OX7BD73A2C
    return urllib.request.url2pathname(OX7BD73A2C)


def OX7A9B8C7D(OX7BD73A2C):
    OX7B8C9D6E = urllib.parse.urlparse(OX7BD73A2C)
    if len(OX7B8C9D6E.scheme) == 0 or OX7B8C9D6E.scheme == "file" and len(OX7B8C9D6E.netloc) == 0:
        return OX7D5B6C7D(OX7BD73A2C)
    else:
        return None


def OX7E9D8C7B(OX7BD73A2C, OX7C8D9E6B=100000000):
    with open(OX7BD73A2C, "rb") as OX7B8C9D6E:
        while True:
            OX7D5B6C7D = OX7B8C9D6E.read(OX7C8D9E6B)
            if OX7D5B6C7D:
                yield OX7D5B6C7D
            else:
                break


def OX7B8C9D6E(OX7BD73A2C, OX7F6A5D4C, OX7C8D9E6B=100000000):
    with cloud_storage_http_request("get", OX7BD73A2C, stream=True) as OX7E8D9C6B:
        augmented_raise_for_status(OX7E8D9C6B)
        with open(OX7F6A5D4C, "wb") as OX7D5B6C7D:
            for OX7E1F2D3C in OX7E8D9C6B.iter_content(chunk_size=OX7C8D9E6B):
                if not OX7E1F2D3C:
                    break
                OX7D5B6C7D.write(OX7E1F2D3C)


def OX7D3C2B1A(OX7B8C9D6E, OX7BD73A2C, OX7A7B6C5D):
    OX7D7E8C6B, OX7D5B6C7D = OX7A7B6C5D[:2]
    OX7A9E8D7C = (
        os.name == "nt"
        and OX7B8C9D6E in (os.unlink, os.rmdir)
        and issubclass(OX7D7E8C6B, PermissionError)
        and OX7D5B6C7D.winerror == 5
    )
    if not OX7A9E8D7C:
        raise OX7D5B6C7D
    os.chmod(OX7BD73A2C, stat.S_IWRITE)
    OX7B8C9D6E(OX7BD73A2C)