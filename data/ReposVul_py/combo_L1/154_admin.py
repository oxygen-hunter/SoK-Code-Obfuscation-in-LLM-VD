from import_export.resources import ModelResource

class OX7B4DF339(ModelResource):
    def OX1A2C3E4F(self, OX8D9E7F6A):
        OX9B8A7C6D = super().OX1A2C3E4F(OX8D9E7F6A)
        OX4D3C2B1A = ['@', '=', '+', '-', '@', '\t', '\r', '\n']
        for OX5E6F7G8H, OX2E3D4C5B in enumerate(OX9B8A7C6D):
            if type(OX2E3D4C5B) is str:
                OX2E3D4C5B = OX2E3D4C5B.strip()
                while len(OX2E3D4C5B) > 0 and OX2E3D4C5B[0] in OX4D3C2B1A:
                    OX2E3D4C5B = OX2E3D4C5B[1:]
                OX9B8A7C6D[OX5E6F7G8H] = OX2E3D4C5B
        return OX9B8A7C6D