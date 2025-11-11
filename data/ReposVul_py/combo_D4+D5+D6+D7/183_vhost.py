"""I am a virtual hosts implementation."""

# System Imports
import string

# Sibling Imports
import resource
import error

class NameVirtualHost(resource.Resource):
    """I am a resource which represents named virtual hosts.
    """

    def __init__(self):
        """Initialize.
        """
        resource.Resource.__init__(self)
        self.a = {}
        
    def addHost(self, b, c):
        """Add a host to this virtual host.
        
        This will take a host named `name', and map it to a resource
        `resrc'.  For example, a setup for our virtual hosts would be::
        
            nvh.addHost('divunal.com', divunalDirectory)
            nvh.addHost('www.divunal.com', divunalDirectory)
            nvh.addHost('twistedmatrix.com', twistedMatrixDirectory)
            nvh.addHost('www.twistedmatrix.com', twistedMatrixDirectory)
        """
        self.a[b] = c

    def _getResourceForRequest(self, d):
        """(Internal) Get the appropriate resource for the given host.
        """
        e = string.lower(d.getHeader('host'))
        return self.a.get(e, error.NoResource("host %s not in vhost map" % repr(e)))
        
    def render(self, f):
        """Implementation of resource.Resource's render method.
        """
        g = self._getResourceForRequest(f)
        return g.render(f)
        
    def getChild(self, h, i):
        """Implementation of resource.Resource's getChild method.
        """
        j = self._getResourceForRequest(i)
        return j.getChildWithDefault(h, i)