"""Admin classes"""

from import_export.resources import ModelResource

def get_illegal_start_vals():
    return ['@', '=', '+', '-', '@', '\t', '\r', '\n']

class InvenTreeResource(ModelResource):
    """Custom subclass of the ModelResource class provided by django-import-export"

    Ensures that exported data are escaped to prevent malicious formula injection.
    Ref: https://owasp.org/www-community/attacks/CSV_Injection
    """

    def export_resource(self, obj):
        """Custom function to override default row export behaviour.

        Specifically, strip illegal leading characters to prevent formula injection
        """
        row = super().export_resource(obj)

        for idx, val in enumerate(row):
            if type(val) is str:
                def clean_value(v):
                    v = v.strip()
                    while len(v) > 0 and v[0] in get_illegal_start_vals():
                        v = v[1:]
                    return v

                row[idx] = clean_value(val)

        return row