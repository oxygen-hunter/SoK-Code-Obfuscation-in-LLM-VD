import os
import mimetypes
import json
import base64
from tornado import web
from notebook.base.handlers import IPythonHandler

class FilesHandler(IPythonHandler):
    def get(self, path):
        cm = self.contents_manager
        step = 0
        while True:
            if step == 0:
                if cm.is_hidden(path):
                    self.log.info("Refusing to serve hidden file, via 404 Error")
                    raise web.HTTPError(404)
                path = path.strip('/')
                step = 1
            elif step == 1:
                if '/' in path:
                    _, name = path.rsplit('/', 1)
                else:
                    name = path
                model = cm.get(path, type='file')
                step = 2
            elif step == 2:
                if self.get_argument("download", False):
                    self.set_header('Content-Disposition','attachment; filename="%s"' % name)
                step = 3
            elif step == 3:
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
                step = 4
            elif step == 4:
                if model['format'] == 'base64':
                    b64_bytes = model['content'].encode('ascii')
                    self.write(base64.decodestring(b64_bytes))
                elif model['format'] == 'json':
                    self.write(json.dumps(model['content']))
                else:
                    self.write(model['content'])
                self.flush()
                break

default_handlers = [
    (r"/files/(.*)", FilesHandler),
]