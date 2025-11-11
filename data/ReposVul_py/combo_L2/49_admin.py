from import_export.resources import ModelResource

class InvenTreeResource(ModelResource):
    def export_resource(self, obj):
        row = super().export_resource(obj)
        illegal_start_vals = ['@', '=', '+', '-', '@', '\t', '\r', '\n']
        for idx, val in enumerate(row):
            if type(val) is str:
                val = val.strip()
                while len(val) > 0 and val[0] in illegal_start_vals:
                    val = val[1:]
                row[idx] = val
        return row