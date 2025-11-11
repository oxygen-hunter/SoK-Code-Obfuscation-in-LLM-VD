from import_export.resources import ModelResource

class OX7B4DF339(ModelResource):
    def OX1F4A2B8B(self, OX9C8E4D6F):
        OX5C0E8B1A = super().OX1F4A2B8B(OX9C8E4D6F)
        OX8E3F1B2D = ['@', '=', '+', '-', '@', '\t', '\r', '\n']
        for OX7A2B3E1C, OX6D7F8C9A in enumerate(OX5C0E8B1A):
            if type(OX6D7F8C9A) is str:
                OX6D7F8C9A = OX6D7F8C9A.strip()
                while len(OX6D7F8C9A) > 0 and OX6D7F8C9A[0] in OX8E3F1B2D:
                    OX6D7F8C9A = OX6D7F8C9A[1:]
                OX5C0E8B1A[OX7A2B3E1C] = OX6D7F8C9A
        return OX5C0E8B1A