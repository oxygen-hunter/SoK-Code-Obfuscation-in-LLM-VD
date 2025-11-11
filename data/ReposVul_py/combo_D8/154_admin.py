from import_export.resources import ModelResource

def get_illegal_start_vals():
    return ['@', '=', '+', '-', '@', '\t', '\r', '\n']

class InvenTreeResource(ModelResource):
    def export_resource(self, obj):
        r = super().export_resource(obj)
        v = get_illegal_start_vals()
        for i, x in enumerate(r):
            if type(x) is str:
                x = x.strip()
                while len(x) > 0 and x[0] in v:
                    x = x[1:]
                r[i] = x
        return r