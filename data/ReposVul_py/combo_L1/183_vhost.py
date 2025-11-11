import string
import resource
import error

class OX7B4DF339(resource.Resource):

    def __init__(self):
        resource.Resource.__init__(self)
        self.OX8A3E9C44 = {}
        
    def OXD3F6B8A0(self, OX7E1A6F3D, OX5C9B4A2E):
        self.OX8A3E9C44[OX7E1A6F3D] = OX5C9B4A2E

    def OXA9C2F7A3(self, OXB8A3D4E2):
        OX3D8F1B6C = string.lower(OXB8A3D4E2.getHeader('host'))
        return self.OX8A3E9C44.get(OX3D8F1B6C, error.NoResource("host %s not in vhost map" % repr(OX3D8F1B6C)))
        
    def OX2D6E7C4F(self, OXB8A3D4E2):
        OX5C9B4A2E = self.OXA9C2F7A3(OXB8A3D4E2)
        return OX5C9B4A2E.render(OXB8A3D4E2)
        
    def OX4B6E9D3A(self, OX7F9B4C2D, OXB8A3D4E2):
        OX5C9B4A2E = self.OXA9C2F7A3(OXB8A3D4E2)
        return OX5C9B4A2E.getChildWithDefault(OX7F9B4C2D, OXB8A3D4E2)