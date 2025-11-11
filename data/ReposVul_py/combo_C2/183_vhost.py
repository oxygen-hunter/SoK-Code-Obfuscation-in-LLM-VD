# System Imports
import string

# Sibling Imports
import resource
import error

class NameVirtualHost(resource.Resource):
    def __init__(self):
        resource.Resource.__init__(self)
        self.hosts = {}
        
    def addHost(self, name, resrc):
        self.hosts[name] = resrc

    def _getResourceForRequest(self, request):
        host = string.lower(request.getHeader('host'))
        return self.hosts.get(host, error.NoResource("host %s not in vhost map" % repr(host)))
        
    def render(self, request):
        state = 0
        while True:
            if state == 0:
                resrc = self._getResourceForRequest(request)
                state = 1
            elif state == 1:
                return resrc.render(request)
        
    def getChild(self, path, request):
        state = 0
        while True:
            if state == 0:
                resrc = self._getResourceForRequest(request)
                state = 1
            elif state == 1:
                return resrc.getChildWithDefault(path, request)