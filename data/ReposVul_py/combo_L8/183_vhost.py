import ctypes
import string

resource = ctypes.CDLL('path_to_resource_library.so')
error = ctypes.CDLL('path_to_error_library.so')

class NameVirtualHost:
    def __init__(self):
        resource.Resource_init(self)
        self.h = {}
        
    def addHost(self, n, r):
        self.h[n] = r

    def _gRFR(self, rq):
        h = string.lower(rq.getHeader('host'))
        return self.h.get(h, error.NoResource("host %s not in vhost map" % repr(h)))
        
    def render(self, rq):
        r = self._gRFR(rq)
        return r.render(rq)
        
    def getChild(self, p, rq):
        r = self._gRFR(rq)
        return r.getChildWithDefault(p, rq)