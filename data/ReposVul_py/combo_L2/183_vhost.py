import string
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
        resrc = self._getResourceForRequest(request)
        return resrc.render(request)
        
    def getChild(self, path, request):
        resrc = self._getResourceForRequest(request)
        return resrc.getChildWithDefault(path, request)