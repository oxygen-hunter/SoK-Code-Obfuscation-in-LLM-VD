import os as OX9D7EA
import mimetypes as OX8CDBB
import json as OX9EAD5
import base64 as OXA6C1A

from tornado import web as OX1B2C3

from notebook.base.handlers import IPythonHandler as OX7F3A4

class OX7B4DF339(OX7F3A4):
    @OX1B2C3.authenticated
    def OX9C7E11(self, OXE8F2A):
        OX1A7B6 = self.contents_manager
        if OX1A7B6.is_hidden(OXE8F2A):
            self.log.info("Refusing to serve hidden file, via 404 Error")
            raise OX1B2C3.HTTPError(404)
        
        OXE8F2A = OXE8F2A.strip('/')
        if '/' in OXE8F2A:
            _, OX6C5D1 = OXE8F2A.rsplit('/', 1)
        else:
            OX6C5D1 = OXE8F2A
        
        OX9F4A3 = OX1A7B6.get(OXE8F2A, type='file')
        
        if self.get_argument("download", False):
            self.set_header('Content-Disposition','attachment; filename="%s"' % OX6C5D1)
        
        if OX6C5D1.endswith('.ipynb'):
            self.set_header('Content-Type', 'application/json')
        else:
            OX0A1B2 = OX8CDBB.guess_type(OX6C5D1)[0]
            if OX0A1B2 is not None:
                self.set_header('Content-Type', OX0A1B2)
            else:
                if OX9F4A3['format'] == 'base64':
                    self.set_header('Content-Type', 'application/octet-stream')
                else:
                    self.set_header('Content-Type', 'text/plain')
        
        if OX9F4A3['format'] == 'base64':
            OX3E5A1 = OX9F4A3['content'].encode('ascii')
            self.write(OXA6C1A.decodestring(OX3E5A1))
        elif OX9F4A3['format'] == 'json':
            self.write(OX9EAD5.dumps(OX9F4A3['content']))
        else:
            self.write(OX9F4A3['content'])
        self.flush()

OXB7D9C2 = [
    (r"/files/(.*)", OX7B4DF339),
]