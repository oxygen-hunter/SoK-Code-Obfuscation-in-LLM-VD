from import_export.resources import ModelResource

class InvenTreeResource(ModelResource):
    def export_resource(self, obj):
        state = 0
        while True:
            if state == 0:
                row = super().export_resource(obj)
                illegal_start_vals = ['@', '=', '+', '-', '@', '\t', '\r', '\n']
                idx = 0
                state = 1
            elif state == 1:
                if idx < len(row):
                    val = row[idx]
                    if type(val) is str:
                        val = val.strip()
                        state = 2
                    else:
                        state = 3
                else:
                    state = 4
            elif state == 2:
                if len(val) > 0 and val[0] in illegal_start_vals:
                    val = val[1:]
                else:
                    row[idx] = val
                    state = 3
            elif state == 3:
                idx += 1
                state = 1
            elif state == 4:
                return row