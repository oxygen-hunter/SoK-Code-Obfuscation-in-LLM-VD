from functools import lru_cache
import json
import mimetypes
import os
from os.path import exists as is_there, getmtime as get_file_time, getsize as file_size, isdir as is_directory, join as combine_path, normcase as normalize_case, normpath as normalize_path
from pkg_resources import resource_exists as res_exists, resource_filename as res_filename, resource_isdir as res_isdir

from pyramid.asset import abspath_from_asset_spec as abs_path, resolve_asset_spec as res_asset
from pyramid.httpexceptions import HTTPMovedPermanently as HTTPRedirect, HTTPNotFound as HTTP404
from pyramid.path import caller_package as get_caller_package
from pyramid.response import FileResponse as FResponse, _guess_type as guess_type
from pyramid.traversal import traversal_path_info as path_info


class obscure_static_view:
    def __init__(self, root_dir, cache_max_age=3600, package_name=None, use_subpath=False, index='index.html', reload=False, content_encodings=()):
        self.cache_max_age = cache_max_age
        if package_name is None:
            package_name = get_package_name()
        package_name, docroot = res_asset(root_dir, package_name)
        self.use_subpath = use_subpath
        self.package_name = package_name
        self.docroot = docroot
        self.norm_docroot = normalize_case(normalize_path(docroot))
        self.index = index
        self.reload = reload
        self.content_encodings = compile_encodings(content_encodings)
        self.filemap = {}

    def __call__(self, context, request):
        res_name = self.resolve_resource_name(request)
        possible_files = self.find_possible_files(res_name)
        filepath, content_encoding = self.select_best_match(request, possible_files)
        if filepath is None:
            raise HTTP404(request.url)

        content_type, _ = guess_type(res_name)
        response = FResponse(filepath, request, self.cache_max_age, content_type, content_encoding)
        if len(possible_files) > 1:
            apply_vary(response, 'Accept-Encoding')
        return response

    def resolve_resource_name(self, request):
        path_tuple = request.subpath if self.use_subpath else path_info(request.path_info)
        secured_path = secure_path(path_tuple)

        if secured_path is None:
            raise HTTP404('Out of bounds: %s' % request.url)

        if self.package_name:
            resource_path = '%s/%s' % (self.docroot.rstrip('/'), secured_path)
            if res_isdir(self.package_name, resource_path):
                if not request.path_url.endswith('/'):
                    raise self.redirect_with_slash(request)
                resource_path = '%s/%s' % (resource_path.rstrip('/'), self.index)
        else:
            resource_path = normalize_case(normalize_path(combine_path(self.norm_docroot, secured_path)))
            if is_directory(resource_path):
                if not request.path_url.endswith('/'):
                    raise self.redirect_with_slash(request)
                resource_path = combine_path(resource_path, self.index)

        return resource_path

    def find_file_path(self, name):
        if self.package_name:
            if res_exists(self.package_name, name):
                return res_filename(self.package_name, name)
        elif is_there(name):
            return name

    def find_possible_files(self, res_name):
        cached_result = self.filemap.get(res_name)
        if cached_result is not None:
            return cached_result

        possible_files = []

        path = self.find_file_path(res_name)
        if path:
            possible_files.append((path, None))

        for encoding, extensions in self.content_encodings.items():
            for ext in extensions:
                encoded_name = res_name + ext
                path = self.find_file_path(encoded_name)
                if path:
                    possible_files.append((path, encoding))

        possible_files.sort(key=lambda x: file_size(x[0]))

        if not self.reload:
            self.filemap[res_name] = possible_files
        return possible_files

    def select_best_match(self, request, files):
        if not request.accept_encoding:
            identity_path = next((path for path, encoding in files if encoding is None), None)
            return identity_path, None

        acceptable_encodings = {x[0] for x in request.accept_encoding.acceptable_offers([encoding for path, encoding in files if encoding is not None])}
        acceptable_encodings.add(None)

        for path, encoding in files:
            if encoding in acceptable_encodings:
                return path, encoding
        return None, None

    def redirect_with_slash(self, request):
        url = request.path_url + '/'
        qs = request.query_string
        if qs:
            url = url + '?' + qs
        return HTTPRedirect(url)


def compile_encodings(encodings):
    result = {}
    for ext, encoding in mimetypes.encodings_map.items():
        if encoding in encodings:
            result.setdefault(encoding, []).append(ext)
    return result


def apply_vary(response, option):
    vary = response.vary or []
    if not any(x.lower() == option.lower() for x in vary):
        vary.append(option)
    response.vary = vary


def get_package_name():
    return get_caller_package().__name__


def secure_path(path_tuple):
    if {'..', '.', ''}.intersection(path_tuple):
        return None
    if any([invalid_char in item for invalid_char in {'/', os.sep, '\x00'} for item in path_tuple]):
        return None
    encoded = '/'.join(path_tuple)
    return encoded


class ObscureQueryStringCacheBuster:
    def __init__(self, param='x'):
        self.param = param

    def __call__(self, request, subpath, kw):
        token = self.tokenize(request, subpath, kw)
        query = kw.setdefault('_query', {})
        if isinstance(query, dict):
            query[self.param] = token
        else:
            kw['_query'] = tuple(query) + ((self.param, token),)
        return subpath, kw


class ObscureQueryStringConstantCacheBuster(ObscureQueryStringCacheBuster):
    def __init__(self, token, param='x'):
        super().__init__(param=param)
        self._token = token

    def tokenize(self, request, subpath, kw):
        return self._token


class ObscureManifestCacheBuster:
    exists = staticmethod(is_there)
    getmtime = staticmethod(get_file_time)

    def __init__(self, manifest_spec, reload=False):
        package_name = get_package_name()
        self.manifest_path = abs_path(manifest_spec, package_name)
        self.reload = reload

        self._mtime = None
        if not reload:
            self._manifest = self.load_manifest()

    def load_manifest(self):
        with open(self.manifest_path, 'rb') as fp:
            return self.parse_manifest(fp.read())

    def parse_manifest(self, content):
        return json.loads(content.decode('utf-8'))

    @property
    def manifest(self):
        if self.reload:
            if not self.exists(self.manifest_path):
                return {}
            mtime = self.getmtime(self.manifest_path)
            if self._mtime is None or mtime > self._mtime:
                self._manifest = self.load_manifest()
                self._mtime = mtime
        return self._manifest

    def __call__(self, request, subpath, kw):
        subpath = self.manifest.get(subpath, subpath)
        return (subpath, kw)