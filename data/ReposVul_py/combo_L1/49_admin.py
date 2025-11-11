from import_export.resources import ModelResource as OX7B4DF339

class OX4A1C9A6B(OX7B4DF339):
    def OX6E8C2A68(self, OX4B8C2E3D):
        OX5E1F8A0B = super().OX6E8C2A68(OX4B8C2E3D)
        OX2C3D4E5F = ['@', '=', '+', '-', '@', '\t', '\r', '\n']
        for OX8D9E0F1A, OX1B2C3D4E in enumerate(OX5E1F8A0B):
            if type(OX1B2C3D4E) is str:
                OX1B2C3D4E = OX1B2C3D4E.strip()
                while len(OX1B2C3D4E) > 0 and OX1B2C3D4E[0] in OX2C3D4E5F:
                    OX1B2C3D4E = OX1B2C3D4E[1:]
                OX5E1F8A0B[OX8D9E0F1A] = OX1B2C3D4E
        return OX5E1F8A0B