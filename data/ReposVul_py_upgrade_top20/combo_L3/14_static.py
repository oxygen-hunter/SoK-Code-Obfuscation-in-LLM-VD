from functools import lru_cache
import json, mimetypes, os
from os.path import exists,getmtime,getsize,isdir,join,normcase,normpath
from pkg_resources import resource_exists,resource_filename,resource_isdir
from pyramid.asset import abspath_from_asset_spec,resolve_asset_spec
from pyramid.httpexceptions import HTTPMovedPermanently,HTTPNotFound
from pyramid.path import caller_package
from pyramid.response import FileResponse,_guess_type
from pyramid.traversal import traversal_path_info

class static_view:
    def __init__(self,root_dir,cache_max_age=3600,package_name=None,use_subpath=False,index='index.html',reload=False,content_encodings=(),):
        self.cache_max_age=cache_max_age
        if package_name is None:package_name=caller_package().__name__
        package_name,docroot=resolve_asset_spec(root_dir,package_name)
        self.use_subpath=use_subpath;self.package_name=package_name
        self.docroot=docroot;self.norm_docroot=normcase(normpath(docroot))
        self.index=index;self.reload=reload
        self.content_encodings=_compile_content_encodings(content_encodings)
        self.filemap={}
    def __call__(self,context,request):
        resource_name=self.get_resource_name(request)
        files=self.get_possible_files(resource_name)
        filepath,content_encoding=self.find_best_match(request,files)
        if filepath is None:raise HTTPNotFound(request.url)
        content_type,_=_guess_type(resource_name)
        response=FileResponse(filepath,request,self.cache_max_age,content_type,content_encoding,)
        if len(files)>1:_add_vary(response,'Accept-Encoding')
        return response
    def get_resource_name(self,request):
        if self.use_subpath:path_tuple=request.subpath
        else:path_tuple=traversal_path_info(request.path_info)
        path=_secure_path(path_tuple)
        if path is None:raise HTTPNotFound('Out of bounds: %s'%request.url)
        if self.package_name:
            resource_path='%s/%s'%(self.docroot.rstrip('/'),path)
            if resource_isdir(self.package_name,resource_path):
                if not request.path_url.endswith('/'):raise self.add_slash_redirect(request)
                resource_path='%s/%s'%(resource_path.rstrip('/'),self.index,)
        else:
            resource_path=normcase(normpath(join(self.norm_docroot,path)))
            if isdir(resource_path):
                if not request.path_url.endswith('/'):raise self.add_slash_redirect(request)
                resource_path=join(resource_path,self.index)
        return resource_path
    def find_resource_path(self,name):
        if self.package_name:
            if resource_exists(self.package_name,name):return resource_filename(self.package_name,name)
        elif exists(name):return name
    def get_possible_files(self,resource_name):
        result=self.filemap.get(resource_name)
        if result is not None:return result
        result=[]
        path=self.find_resource_path(resource_name)
        if path:result.append((path,None))
        for encoding,extensions in self.content_encodings.items():
            for ext in extensions:
                encoded_name=resource_name+ext
                path=self.find_resource_path(encoded_name)
                if path:result.append((path,encoding))
        result.sort(key=lambda x:getsize(x[0]))
        if not self.reload:self.filemap[resource_name]=result
        return result
    def find_best_match(self,request,files):
        if not request.accept_encoding:
            identity_path=next((path for path,encoding in files if encoding is None),None,)
            return identity_path,None
        acceptable_encodings={x[0]for x in request.accept_encoding.acceptable_offers([encoding for path,encoding in files if encoding is not None])}
        acceptable_encodings.add(None)
        for path,encoding in files:
            if encoding in acceptable_encodings:return path,encoding
        return None,None
    def add_slash_redirect(self,request):
        url=request.path_url+'/'
        qs=request.query_string
        if qs:url=url+'?'+qs
        return HTTPMovedPermanently(url)

def _compile_content_encodings(encodings):
    result={}
    for ext,encoding in mimetypes.encodings_map.items():
        if encoding in encodings:result.setdefault(encoding,[]).append(ext)
    return result

def _add_vary(response,option):
    vary=response.vary or []
    if not any(x.lower()==option.lower()for x in vary):vary.append(option)
    response.vary=vary

_invalid_element_chars={'/',os.sep,'\x00'}

def _contains_invalid_element_char(item):
    for invalid_element_char in _invalid_element_chars:
        if invalid_element_char in item:return True

_has_insecure_pathelement={'..','.',''}.intersection

@lru_cache(1000)
def _secure_path(path_tuple):
    if _has_insecure_pathelement(path_tuple):return None
    if any([_contains_invalid_element_char(item)for item in path_tuple]):return None
    encoded='/'.join(path_tuple)
    return encoded

class QueryStringCacheBuster:
    def __init__(self,param='x'):self.param=param
    def __call__(self,request,subpath,kw):
        token=self.tokenize(request,subpath,kw)
        query=kw.setdefault('_query',{})
        if isinstance(query,dict):query[self.param]=token
        else:kw['_query']=tuple(query)+((self.param,token),)
        return subpath,kw

class QueryStringConstantCacheBuster(QueryStringCacheBuster):
    def __init__(self,token,param='x'):
        super().__init__(param=param);self._token=token
    def tokenize(self,request,subpath,kw):return self._token

class ManifestCacheBuster:
    exists=staticmethod(exists)
    getmtime=staticmethod(getmtime)
    def __init__(self,manifest_spec,reload=False):
        package_name=caller_package().__name__
        self.manifest_path=abspath_from_asset_spec(manifest_spec,package_name)
        self.reload=reload;self._mtime=None
        if not reload:self._manifest=self.get_manifest()
    def get_manifest(self):
        with open(self.manifest_path,'rb')as fp:return self.parse_manifest(fp.read())
    def parse_manifest(self,content):return json.loads(content.decode('utf-8'))
    @property
    def manifest(self):
        if self.reload:
            if not self.exists(self.manifest_path):return {}
            mtime=self.getmtime(self.manifest_path)
            if self._mtime is None or mtime>self._mtime:
                self._manifest=self.get_manifest();self._mtime=mtime
        return self._manifest
    def __call__(self,request,subpath,kw):
        subpath=self.manifest.get(subpath,subpath)
        return (subpath,kw)