import os
import mimetypes
import json
import base64

from tornado import web

from notebook.base.handlers import IPythonHandler

class FilesHandler(IPythonHandler):
    """serve files via ContentsManager"""

    @web.authenticated
    def get(self, _path):
        cm = self.contents_manager
        if cm.is_hidden(_path):
            self.log.info("Refusing to serve hidden file, via 404 Error")
            raise web.HTTPError(404)
        
        _path = _path.strip('/')
        p = _path.rsplit('/', 1) if '/' in _path else [None, _path]
        
        n = p[1]
        mdl = cm.get(_path, type='file')
        
        arg_dwnld = self.get_argument("download", False)
        if arg_dwnld:
            self.set_header('Content-Disposition','attachment; filename="%s"' % n)
        
        if n.endswith('.ipynb'):
            self.set_header('Content-Type', 'application/json')
        else:
            c_m = mimetypes.guess_type(n)[0]
            if c_m is not None:
                self.set_header('Content-Type', c_m)
            else:
                if mdl['format'] == 'base64':
                    self.set_header('Content-Type', 'application/octet-stream')
                else:
                    self.set_header('Content-Type', 'text/plain')
        
        if mdl['format'] == 'base64':
            b = mdl['content'].encode('ascii')
            self.write(base64.decodestring(b))
        elif mdl['format'] == 'json':
            self.write(json.dumps(mdl['content']))
        else:
            self.write(mdl['content'])
        self.flush()

_hndlr = [
    (r"/files/(.*)", FilesHandler),
]