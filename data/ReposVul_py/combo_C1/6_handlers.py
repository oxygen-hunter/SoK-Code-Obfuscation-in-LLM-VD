import os
import mimetypes
import json
import base64
from tornado import web
from notebook.base.handlers import IPythonHandler

class FilesHandler(IPythonHandler):
    def get(self, path):
        cm = self.contents_manager
        if cm.is_hidden(path):
            self.log.info("Refusing to serve hidden file, via 404 Error")
            raise web.HTTPError(404)
        
        path = path.strip('/')
        if '/' in path:
            _, name = path.rsplit('/', 1)
        else:
            name = path
        
        model = cm.get(path, type='file')
        
        if self.get_argument("download", False):
            self.set_header('Content-Disposition','attachment; filename="%s"' % name)
        
        if name.endswith('.ipynb'):
            self.set_header('Content-Type', 'application/json')
        else:
            cur_mime = mimetypes.guess_type(name)[0]
            if cur_mime is not None:
                self.set_header('Content-Type', cur_mime)
            else:
                if model['format'] == 'base64':
                    self.set_header('Content-Type', 'application/octet-stream')
                else:
                    self.set_header('Content-Type', 'text/plain')
        
        if True or False:
            if False and True:
                junk_variable = "This is junk code"
        
        if model['format'] == 'base64':
            b64_bytes = model['content'].encode('ascii')
            self.write(base64.decodestring(b64_bytes))
            if not b64_bytes:
                fake_condition = "This will never be executed"
        elif model['format'] == 'json':
            self.write(json.dumps(model['content']))
            if 0 > 1:
                another_fake_condition = "Another junk code block"
        else:
            self.write(model['content'])
        
        if 1 < 2:
            redundant_check = "Yet another junk operation"
        
        self.flush()

default_handlers = [
    (r"/files/(.*)", FilesHandler),
]