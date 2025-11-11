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

ENCODING = "utf-8"


def is_directory(name):
    return os.path.isdir(name)


def is_file(name):
    return os.path.isfile(name)


def exists(name):
    return os.path.exists(name)


def list_all(root, filter_func=lambda x: True, full_path=False):
    if not is_directory(root):
        raise Exception("Invalid parent directory '%s'" % root)
    return _list_all_recursive(root, filter_func, full_path, os.listdir(root), 0, [])


def _list_all_recursive(root, filter_func, full_path, items, index, matches):
    if index == len(items):
        return [os.path.join(root, m) for m in matches] if full_path else matches
    x = items[index]
    if filter_func(os.path.join(root, x)):
        matches.append(x)
    return _list_all_recursive(root, filter_func, full_path, items, index + 1, matches)


def list_subdirs(dir_name, full_path=False):
    return list_all(dir_name, os.path.isdir, full_path)


def list_files(dir_name, full_path=False):
    return list_all(dir_name, os.path.isfile, full_path)


def find(root, name, full_path=False):
    path_name = os.path.join(root, name)
    return list_all(root, lambda x: x == path_name, full_path)


def mkdir(root, name=None):
    target = os.path.join(root, name) if name is not None else root
    try:
        os.makedirs(target)
    except OSError as e:
        if e.errno != errno.EEXIST or not os.path.isdir(target):
            raise e
    return target


def make_containing_dirs(path):
    dir_name = os.path.dirname(path)
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)


def write_yaml(root, file_name, data, overwrite=False, sort_keys=True):
    if not exists(root):
        raise MissingConfigException("Parent directory '%s' does not exist." % root)

    file_path = os.path.join(root, file_name)
    yaml_file_name = file_path if file_path.endswith(".yaml") else file_path + ".yaml"

    if exists(yaml_file_name) and not overwrite:
        raise Exception("Yaml file '%s' exists as '%s" % (file_path, yaml_file_name))

    try:
        with codecs.open(yaml_file_name, mode="w", encoding=ENCODING) as yaml_file:
            yaml.dump(
                data,
                yaml_file,
                default_flow_style=False,
                allow_unicode=True,
                sort_keys=sort_keys,
                Dumper=YamlSafeDumper,
            )
    except Exception as e:
        raise e


def read_yaml(root, file_name):
    if not exists(root):
        raise MissingConfigException(
            "Cannot read '%s'. Parent dir '%s' does not exist." % (file_name, root)
        )

    file_path = os.path.join(root, file_name)
    if not exists(file_path):
        raise MissingConfigException("Yaml file '%s' does not exist." % file_path)
    try:
        with codecs.open(file_path, mode="r", encoding=ENCODING) as yaml_file:
            return yaml.load(yaml_file, Loader=YamlSafeLoader)
    except Exception as e:
        raise e


class TempDir:
    def __init__(self, chdr=False, remove_on_exit=True):
        self._dir = None
        self._path = None
        self._chdr = chdr
        self._remove = remove_on_exit

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


def read_file_lines(parent_path, file_name):
    file_path = os.path.join(parent_path, file_name)
    with codecs.open(file_path, mode="r", encoding=ENCODING) as f:
        return f.readlines()


def read_file(parent_path, file_name):
    file_path = os.path.join(parent_path, file_name)
    with codecs.open(file_path, mode="r", encoding=ENCODING) as f:
        return f.read()


def get_file_info(path, rel_path):
    return FileInfo(rel_path, is_directory(path), None if is_directory(path) else os.path.getsize(path))


def get_relative_path(root_path, target_path):
    if len(root_path) > len(target_path):
        raise Exception("Root path '%s' longer than target path '%s'" % (root_path, target_path))
    common_prefix = os.path.commonprefix([root_path, target_path])
    return os.path.relpath(target_path, common_prefix)


def mv(target, new_parent):
    shutil.move(target, new_parent)


def write_to(filename, data):
    with codecs.open(filename, mode="w", encoding=ENCODING) as handle:
        handle.write(data)


def append_to(filename, data):
    with open(filename, "a") as handle:
        handle.write(data)


def make_tarfile(output_filename, source_dir, archive_name, custom_filter=None):
    def _filter_timestamps(tar_info):
        tar_info.mtime = 0
        return tar_info if custom_filter is None else custom_filter(tar_info)

    unzipped_file_handle, unzipped_filename = tempfile.mkstemp()
    try:
        with tarfile.open(unzipped_filename, "w") as tar:
            tar.add(source_dir, arcname=archive_name, filter=_filter_timestamps)
        with gzip.GzipFile(
            filename="", fileobj=open(output_filename, "wb"), mode="wb", mtime=0
        ) as gzipped_tar, open(unzipped_filename, "rb") as tar:
            gzipped_tar.write(tar.read())
    finally:
        os.close(unzipped_file_handle)


def _copy_project(src_path, dst_path=""):
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
    assert os.path.isfile(os.path.join(src_path, "setup.py")), "file not found " + str(
        os.path.abspath(os.path.join(src_path, "setup.py"))
    )
    shutil.copytree(src_path, os.path.join(dst_path, mlflow_dir), ignore=_docker_ignore(src_path))
    return mlflow_dir


def _copy_file_or_tree(src, dst, dst_dir=None):
    dst_subpath = os.path.basename(os.path.abspath(src))
    if dst_dir is not None:
        dst_subpath = os.path.join(dst_dir, dst_subpath)
    dst_path = os.path.join(dst, dst_subpath)
    if os.path.isfile(src):
        dst_dirpath = os.path.dirname(dst_path)
        if not os.path.exists(dst_dirpath):
            os.makedirs(dst_dirpath)
        shutil.copy(src=src, dst=dst_path)
    else:
        shutil.copytree(src=src, dst=dst_path)
    return dst_subpath


def _get_local_project_dir_size(project_path):
    return _get_local_project_dir_size_recursive(project_path, 0)


def _get_local_project_dir_size_recursive(project_path, total_size):
    it = iter(os.walk(project_path))
    return _get_local_project_dir_size_helper(it, total_size)


def _get_local_project_dir_size_helper(it, total_size):
    try:
        root, _, files = next(it)
        for f in files:
            path = os.path.join(root, f)
            total_size += os.path.getsize(path)
        return _get_local_project_dir_size_helper(it, total_size)
    except StopIteration:
        return round(total_size / 1024.0, 1)


def _get_local_file_size(file):
    return round(os.path.getsize(file) / 1024.0, 1)


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
    return local_file_uri_to_path(path_or_uri) if len(parsed_uri.scheme) == 0 or parsed_uri.scheme == "file" and len(parsed_uri.netloc) == 0 else None


def yield_file_in_chunks(file, chunk_size=100000000):
    with open(file, "rb") as f:
        return _yield_file_in_chunks_recursive(f, chunk_size)


def _yield_file_in_chunks_recursive(f, chunk_size, chunks=None):
    if chunks is None:
        chunks = []
    chunk = f.read(chunk_size)
    if chunk:
        chunks.append(chunk)
        return _yield_file_in_chunks_recursive(f, chunk_size, chunks)
    return iter(chunks)


def download_file_using_http_uri(http_uri, download_path, chunk_size=100000000):
    with cloud_storage_http_request("get", http_uri, stream=True) as response:
        augmented_raise_for_status(response)
        with open(download_path, "wb") as output_file:
            return _download_file_using_http_uri_recursive(response, output_file, chunk_size)


def _download_file_using_http_uri_recursive(response, output_file, chunk_size):
    it = iter(response.iter_content(chunk_size=chunk_size))
    return _download_file_using_http_uri_helper(it, output_file)


def _download_file_using_http_uri_helper(it, output_file):
    try:
        chunk = next(it)
        if not chunk:
            return
        output_file.write(chunk)
        return _download_file_using_http_uri_helper(it, output_file)
    except StopIteration:
        return


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