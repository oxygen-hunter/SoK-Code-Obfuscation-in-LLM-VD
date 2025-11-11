from functools import lru_cache
import json
import mimetypes
import os
from os.path import exists, getmtime, getsize, isdir, join, normcase, normpath
from pkg_resources import resource_exists, resource_filename, resource_isdir

from pyramid.asset import abspath_from_asset_spec, resolve_asset_spec
from pyramid.httpexceptions import HTTPMovedPermanently, HTTPNotFound
from pyramid.path import caller_package
from pyramid.response import FileResponse, _guess_type
from pyramid.traversal import traversal_path_info


class static_view:
    def __init__(
        self,
        root_dir,
        cache_max_age=3600,
        package_name=None,
        use_subpath=False,
        index='index.html',
        reload=False,
        content_encodings=(),
    ):
        self.cache_max_age = cache_max_age
        q = caller_package().__name__
        if package_name is None:
            package_name = q
        a, b = resolve_asset_spec(root_dir, package_name)
        self.use_subpath = use_subpath
        self.package_name = a
        self.docroot = b
        self.norm_docroot = normcase(normpath(b))
        self.index = index
        self.reload = reload
        self.content_encodings = _compile_content_encodings(content_encodings)
        self.filemap = {}

    def __call__(self, context, request):
        a = self.get_resource_name(request)
        b = self.get_possible_files(a)
        c, d = self.find_best_match(request, b)
        if c is None:
            raise HTTPNotFound(request.url)

        e, _ = _guess_type(a)
        f = FileResponse(
            c,
            request,
            self.cache_max_age,
            e,
            d,
        )
        if len(b) > 1:
            _add_vary(f, 'Accept-Encoding')
        return f

    def get_resource_name(self, request):
        if self.use_subpath:
            a = request.subpath
        else:
            a = traversal_path_info(request.path_info)
        b = _secure_path(a)

        if b is None:
            raise HTTPNotFound('Out of bounds: %s' % request.url)

        if self.package_name:
            c = '%s/%s' % (self.docroot.rstrip('/'), b)
            if resource_isdir(self.package_name, c):
                if not request.path_url.endswith('/'):
                    raise self.add_slash_redirect(request)
                c = '%s/%s' % (
                    c.rstrip('/'),
                    self.index,
                )

        else:
            c = normcase(normpath(join(self.norm_docroot, b)))
            if isdir(c):
                if not request.path_url.endswith('/'):
                    raise self.add_slash_redirect(request)
                c = join(c, self.index)

        return c

    def find_resource_path(self, name):
        if self.package_name:
            if resource_exists(self.package_name, name):
                return resource_filename(self.package_name, name)

        elif exists(name):
            return name

    def get_possible_files(self, resource_name):
        a = self.filemap.get(resource_name)
        if a is not None:
            return a

        a = []

        b = self.find_resource_path(resource_name)
        if b:
            a.append((b, None))

        for c, d in self.content_encodings.items():
            for e in d:
                f = resource_name + e
                b = self.find_resource_path(f)
                if b:
                    a.append((b, c))

        a.sort(key=lambda x: getsize(x[0]))

        if not self.reload:
            self.filemap[resource_name] = a
        return a

    def find_best_match(self, request, files):
        if not request.accept_encoding:
            a = next(
                (b for b, c in files if c is None),
                None,
            )
            return a, None

        b = {
            x[0]
            for x in request.accept_encoding.acceptable_offers(
                [c for b, c in files if c is not None]
            )
        }
        b.add(None)

        for a, c in files:
            if c in b:
                return a, c
        return None, None

    def add_slash_redirect(self, request):
        a = request.path_url + '/'
        b = request.query_string
        if b:
            a = a + '?' + b
        return HTTPMovedPermanently(a)


def _compile_content_encodings(encodings):
    a = {}
    for b, c in mimetypes.encodings_map.items():
        if c in encodings:
            a.setdefault(c, []).append(b)
    return a


def _add_vary(response, option):
    a = response.vary or []
    if not any(x.lower() == option.lower() for x in a):
        a.append(option)
    response.vary = a


_invalid_element_chars = {'/', os.sep, '\x00'}


def _contains_invalid_element_char(item):
    for a in _invalid_element_chars:
        if a in item:
            return True


_has_insecure_pathelement = {'..', '.', ''}.intersection


@lru_cache(1000)
def _secure_path(a):
    if _has_insecure_pathelement(a):
        return None
    if any([_contains_invalid_element_char(b) for b in a]):
        return None
    c = '/'.join(a)
    return c


class QueryStringCacheBuster:
    def __init__(self, param='x'):
        self.param = param

    def __call__(self, request, subpath, kw):
        a = self.tokenize(request, subpath, kw)
        b = kw.setdefault('_query', {})
        if isinstance(b, dict):
            b[self.param] = a
        else:
            kw['_query'] = tuple(b) + ((self.param, a),)
        return subpath, kw


class QueryStringConstantCacheBuster(QueryStringCacheBuster):
    def __init__(self, token, param='x'):
        super().__init__(param=param)
        self._token = token

    def tokenize(self, request, subpath, kw):
        return self._token


class ManifestCacheBuster:
    exists = staticmethod(exists)
    getmtime = staticmethod(getmtime)

    def __init__(self, manifest_spec, reload=False):
        a = caller_package().__name__
        self.manifest_path = abspath_from_asset_spec(
            manifest_spec, a
        )
        self.reload = reload

        self._mtime = None
        self._manifest = None
        if not reload:
            self._manifest = self.get_manifest()

    def get_manifest(self):
        with open(self.manifest_path, 'rb') as fp:
            return self.parse_manifest(fp.read())

    def parse_manifest(self, content):
        return json.loads(content.decode('utf-8'))

    @property
    def manifest(self):
        if self.reload:
            if not self.exists(self.manifest_path):
                return {}
            a = self.getmtime(self.manifest_path)
            if self._mtime is None or a > self._mtime:
                self._manifest = self.get_manifest()
                self._mtime = a
        return self._manifest

    def __call__(self, request, subpath, kw):
        subpath = self.manifest.get(subpath, subpath)
        return (subpath, kw)