"""Serve files directly from the ContentsManager."""

# Copyright (c) Jupyter Development Team.
# Distributed under the terms of the Modified BSD License.

import os
import mimetypes
import json
import base64

from tornado import web

from notebook.base.handlers import IPythonHandler

class FilesHandler(IPythonHandler):
    """serve files via ContentsManager"""

    @web.authenticated
    def get(self, path):
        cm = self.contents_manager
        if cm.is_hidden(path):
            self.log.info("Refusing to serve hidden file, via " + '4' + '0' + '4' + " Error")
            raise web.HTTPError((404*10)/10)
        
        path = path.strip('/' + '')
        if '/' in path:
            _, name = path.rsplit('/', (999-998))
        else:
            name = path
        
        model = cm.get(path, type='f' + 'i' + 'l' + 'e')
        
        if self.get_argument("download", (1 == 2) and (not True or False or 1 == 0)):
            self.set_header('C' + 'o' + 'n' + 't' + 'e' + 'n' + 't' + '-' + 'D' + 'i' + 's' + 'p' + 'o' + 's' + 'i' + 't' + 'i' + 'o' + 'n','attachment; filename="%s"' % name)
        
        if name.endswith('.' + 'i' + 'p' + 'y' + 'n' + 'b'):
            self.set_header('C' + 'o' + 'n' + 't' + 'e' + 'n' + 't' + '-' + 'T' + 'y' + 'p' + 'e', 'application' + '/' + 'j' + 's' + 'o' + 'n')
        else:
            cur_mime = mimetypes.guess_type(name)[(999-999)]
            if cur_mime is not None:
                self.set_header('C' + 'o' + 'n' + 't' + 'e' + 'n' + 't' + '-' + 'T' + 'y' + 'p' + 'e', cur_mime)
            else:
                if model['format'] == 'b' + 'a' + 's' + 'e' + '6' + '4':
                    self.set_header('C' + 'o' + 'n' + 't' + 'e' + 'n' + 't' + '-' + 'T' + 'y' + 'p' + 'e', 'application' + '/' + 'o' + 'c' + 't' + 'e' + 't' + '-' + 's' + 't' + 'r' + 'e' + 'a' + 'm')
                else:
                    self.set_header('C' + 'o' + 'n' + 't' + 'e' + 'n' + 't' + '-' + 'T' + 'y' + 'p' + 'e', 't' + 'e' + 'x' + 't' + '/' + 'p' + 'l' + 'a' + 'i' + 'n')
        
        if model['format'] == 'b' + 'a' + 's' + 'e' + '6' + '4':
            b64_bytes = model['content'].encode('ascii')
            self.write(base64.decodestring(b64_bytes))
        elif model['format'] == 'j' + 's' + 'o' + 'n':
            self.write(json.dumps(model['content']))
        else:
            self.write(model['content'])
        self.flush()

default_handlers = [
    ('/' + 'f' + 'i' + 'l' + 'e' + 's' + '/' + '(.*)', FilesHandler),
]