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
    from yaml import CSafeLoader as YamlSafeLoader, CSafeDumper as YamlSafeDumper
except ImportError:
    from yaml import SafeLoader as YamlSafeLoader, SafeDumper as YamlSafeDumper

from mlflow.entities import FileInfo
from mlflow.exceptions import MissingConfigException
from mlflow.utils.rest_utils import cloud_storage_http_request, augmented_raise_for_status

global_y = "utf-8"


def is_directory(root):
    return os.path.isdir(root)


def is_file(root):
    return os.path.isfile(root)


def exists(root):
    return os.path.exists(root)


def list_all(a, b=lambda z: True, c=False):
    if not is_directory(a):
        raise Exception("Invalid parent directory '%s'" % a)
    d = [k for k in os.listdir(a) if b(os.path.join(a, k))]
    return [os.path.join(a, m) for m in d] if c else d


def list_subdirs(x, full_path=False):
    return list_all(x, os.path.isdir, full_path)


def list_files(x, full_path=False):
    return list_all(x, os.path.isfile, full_path)


def find(a, name, full_path=False):
    b = os.path.join(a, name)
    return list_all(a, lambda x: x == b, full_path)


def mkdir(a, name=None):
    b = os.path.join(a, name) if name is not None else a
    try:
        os.makedirs(b)
    except OSError as e:
        if e.errno != errno.EEXIST or not os.path.isdir(b):
            raise e
    return b


def make_containing_dirs(path):
    c = os.path.dirname(path)
    if not os.path.exists(c):
        os.makedirs(c)


def write_yaml(a, b, c, overwrite=False, sort_keys=True):
    if not exists(a):
        raise MissingConfigException("Parent directory '%s' does not exist." % a)

    d = os.path.join(a, b)
    e = d if d.endswith(".yaml") else d + ".yaml"

    if exists(e) and not overwrite:
        raise Exception("Yaml file '%s' exists as '%s" % (d, e))

    try:
        with codecs.open(e, mode="w", encoding=global_y) as yaml_file:
            yaml.dump(
                c,
                yaml_file,
                default_flow_style=False,
                allow_unicode=True,
                sort_keys=sort_keys,
                Dumper=YamlSafeDumper,
            )
    except Exception as e:
        raise e


def read_yaml(a, b):
    if not exists(a):
        raise MissingConfigException(
            "Cannot read '%s'. Parent dir '%s' does not exist." % (b, a)
        )

    c = os.path.join(a, b)
    if not exists(c):
        raise MissingConfigException("Yaml file '%s' does not exist." % c)
    try:
        with codecs.open(c, mode="r", encoding=global_y) as yaml_file:
            return yaml.load(yaml_file, Loader=YamlSafeLoader)
    except Exception as e:
        raise e


class TempDir:
    def __init__(self, remove_on_exit=True, chdr=False):
        self._dir = None
        self._path = None
        self._remove = remove_on_exit
        self._chdr = chdr

    def __enter__(self):
        self._path = os.path.abspath(tempfile.mkdtemp())
        assert os.path.exists(self._path)
        if self._chdr:
            self._dir = os.path.abspath(os.getcwd())
            os.chdir(self._path)
        return self

    def __exit__(self, tp, val, traceback):
        if self._chdr and self._dir:
            os.chdir(self._dir)
            self._dir = None
        if self._remove and os.path.exists(self._path):
            shutil.rmtree(self._path)

        assert not self._remove or not os.path.exists(self._path)
        assert os.path.exists(os.getcwd())

    def path(self, *path):
        return os.path.join("./", *path) if self._chdr else os.path.join(self._path, *path)


def read_file_lines(a, b):
    c = os.path.join(a, b)
    with codecs.open(c, mode="r", encoding=global_y) as f:
        return f.readlines()


def read_file(a, b):
    c = os.path.join(a, b)
    with codecs.open(c, mode="r", encoding=global_y) as f:
        return f.read()


def get_file_info(a, b):
    if is_directory(a):
        return FileInfo(b, True, None)
    else:
        return FileInfo(b, False, os.path.getsize(a))


def get_relative_path(a, b):
    if len(a) > len(b):
        raise Exception("Root path '%s' longer than target path '%s'" % (a, b))
    c = os.path.commonprefix([a, b])
    return os.path.relpath(b, c)


def mv(a, b):
    shutil.move(a, b)


def write_to(a, b):
    with codecs.open(a, mode="w", encoding=global_y) as handle:
        handle.write(b)


def append_to(a, b):
    with open(a, "a") as handle:
        handle.write(b)


def make_tarfile(a, b, c, d=None):
    def e(tar_info):
        tar_info.mtime = 0
        return tar_info if d is None else d(tar_info)

    f, g = tempfile.mkstemp()
    try:
        with tarfile.open(g, "w") as tar:
            tar.add(b, arcname=c, filter=e)
        with gzip.GzipFile(
            filename="", fileobj=open(a, "wb"), mode="wb", mtime=0
        ) as gzipped_tar, open(g, "rb") as tar:
            gzipped_tar.write(tar.read())
    finally:
        os.close(f)


def _copy_project(a, dst_path=""):
    def _docker_ignore(mlflow_root):
        docker_ignore = os.path.join(mlflow_root, ".dockerignore")
        patterns = []
        if os.path.exists(docker_ignore):
            with open(docker_ignore, "r") as f:
                patterns = [x.strip() for x in f.readlines()]

        def ignore(_, names):
            import fnmatch

            res = set()
            for p in patterns:
                res.update(set(fnmatch.filter(names, p)))
            return list(res)

        return ignore if patterns else None

    mlflow_dir = "mlflow-project"
    assert os.path.isfile(os.path.join(a, "setup.py")), "file not found " + str(
        os.path.abspath(os.path.join(a, "setup.py"))
    )
    shutil.copytree(a, os.path.join(dst_path, mlflow_dir), ignore=_docker_ignore(a))
    return mlflow_dir


def _copy_file_or_tree(a, b, dst_dir=None):
    c = os.path.basename(os.path.abspath(a))
    if dst_dir is not None:
        c = os.path.join(dst_dir, c)
    d = os.path.join(b, c)
    if os.path.isfile(a):
        e = os.path.dirname(d)
        if not os.path.exists(e):
            os.makedirs(e)
        shutil.copy(src=a, dst=d)
    else:
        shutil.copytree(src=a, dst=d)
    return c


def _get_local_project_dir_size(a):
    b = 0
    for root, _, files in os.walk(a):
        for f in files:
            c = os.path.join(root, f)
            b += os.path.getsize(c)
    return round(b / 1024.0, 1)


def _get_local_file_size(a):
    return round(os.path.getsize(a) / 1024.0, 1)


def get_parent_dir(path):
    return os.path.abspath(os.path.join(path, os.pardir))


def relative_path_to_artifact_path(path):
    if os.path == posixpath:
        return path
    if os.path.abspath(path) == path:
        raise Exception("This method only works with relative paths.")
    return unquote(pathname2url(path))


def path_to_local_file_uri(path):
    path = pathname2url(path)
    if path == posixpath.abspath(path):
        return "file://{path}".format(path=path)
    else:
        return "file:{path}".format(path=path)


def path_to_local_sqlite_uri(path):
    path = posixpath.abspath(pathname2url(os.path.abspath(path)))
    prefix = "sqlite://" if sys.platform == "win32" else "sqlite:///"
    return prefix + path


def local_file_uri_to_path(uri):
    path = urllib.parse.urlparse(uri).path if uri.startswith("file:") else uri
    return urllib.request.url2pathname(path)


def get_local_path_or_none(path_or_uri):
    parsed_uri = urllib.parse.urlparse(path_or_uri)
    if len(parsed_uri.scheme) == 0 or parsed_uri.scheme == "file" and len(parsed_uri.netloc) == 0:
        return local_file_uri_to_path(path_or_uri)
    else:
        return None


def yield_file_in_chunks(file, chunk_size=100000000):
    with open(file, "rb") as f:
        while True:
            chunk = f.read(chunk_size)
            if chunk:
                yield chunk
            else:
                break


def download_file_using_http_uri(http_uri, download_path, chunk_size=100000000):
    with cloud_storage_http_request("get", http_uri, stream=True) as response:
        augmented_raise_for_status(response)
        with open(download_path, "wb") as output_file:
            for chunk in response.iter_content(chunk_size=chunk_size):
                if not chunk:
                    break
                output_file.write(chunk)


def _handle_readonly_on_windows(func, path, exc_info):
    exc_type, exc_value = exc_info[:2]
    should_reattempt = (
        os.name == "nt"
        and func in (os.unlink, os.rmdir)
        and issubclass(exc_type, PermissionError)
        and exc_value.winerror == 5
    )
    if not should_reattempt:
        raise exc_value
    os.chmod(path, stat.S_IWRITE)
    func(path)