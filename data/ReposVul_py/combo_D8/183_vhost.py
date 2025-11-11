# System Imports
import string

# Sibling Imports
import resource
import error

class NameVirtualHost(resource.Resource):
    def __init__(self):
        resource.Resource.__init__(self)
        self._dynamicDataStore = {}

    def _getDynamicData(self, key):
        return self._dynamicDataStore.get(key)

    def _setDynamicData(self, key, value):
        self._dynamicDataStore[key] = value

    def addHost(self, name, resrc):
        self._setDynamicData(name, resrc)

    def _getResourceForRequest(self, request):
        host = string.lower(request.getHeader('host'))
        return self._getDynamicData(host) or error.NoResource("host %s not in vhost map" % repr(host))
        
    def render(self, request):
        resrc = self._getResourceForRequest(request)
        return resrc.render(request)
        
    def getChild(self, path, request):
        resrc = self._getResourceForRequest(request)
        return resrc.getChildWithDefault(path, request)