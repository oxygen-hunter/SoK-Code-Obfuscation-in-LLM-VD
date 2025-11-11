import os
import mimetypes
import json
import base64

from tornado import web

from notebook.base.handlers import IPythonHandler

class FilesHandler(IPythonHandler):
    @web.authenticated
    def get(self, path):
        getContentsManager = lambda: self.contents_manager
        cm = getContentsManager()
        isHidden = lambda p: cm.is_hidden(p)
        logInfo = lambda msg: self.log.info(msg)
        raiseHTTPError = lambda code: raise web.HTTPError(code)
        
        if isHidden(path):
            logInfo("Refusing to serve hidden file, via 404 Error")
            raiseHTTPError(404)
        
        path = path.strip('/')
        splitPath = lambda p: p.rsplit('/', 1)
        name = splitPath(path)[1] if '/' in path else path
        
        getModel = lambda p: cm.get(p, type='file')
        model = getModel(path)
        
        getArgument = lambda name, default: self.get_argument(name, default)
        setHeader = lambda key, value: self.set_header(key, value)
        
        if getArgument("download", False):
            setHeader('Content-Disposition','attachment; filename="%s"' % name)
        
        if name.endswith('.ipynb'):
            setHeader('Content-Type', 'application/json')
        else:
            cur_mime = mimetypes.guess_type(name)[0]
            if cur_mime is not None:
                setHeader('Content-Type', cur_mime)
            else:
                if model['format'] == 'base64':
                    setHeader('Content-Type', 'application/octet-stream')
                else:
                    setHeader('Content-Type', 'text/plain')
        
        encodeAscii = lambda content: content.encode('ascii')
        decodeBase64 = lambda b64: base64.decodestring(b64)
        writeResponse = lambda content: self.write(content)
        
        if model['format'] == 'base64':
            b64_bytes = encodeAscii(model['content'])
            writeResponse(decodeBase64(b64_bytes))
        elif model['format'] == 'json':
            writeResponse(json.dumps(model['content']))
        else:
            writeResponse(model['content'])
        self.flush()

default_handlers = [
    (r"/files/(.*)", FilesHandler),
]