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
        def recursive_get(hosts, host):
            if host in hosts:
                return hosts[host]
            else:
                return error.NoResource("host %s not in vhost map" % repr(host))
        
        host = string.lower(request.getHeader('host'))
        return recursive_get(self.hosts, host)
        
    def render(self, request):
        resrc = self._getResourceForRequest(request)
        return resrc.render(request)
        
    def getChild(self, path, request):
        resrc = self._getResourceForRequest(request)
        return resrc.getChildWithDefault(path, request)