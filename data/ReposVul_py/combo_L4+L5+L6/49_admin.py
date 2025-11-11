"""Admin classes"""

from import_export.resources import ModelResource

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

        illegal_start_vals = ['@', '=', '+', '-', '@', '\t', '\r', '\n']

        def clean_value(val, illegal_start_vals):
            if len(val) > 0 and val[0] in illegal_start_vals:
                return clean_value(val[1:], illegal_start_vals)
            return val

        def process_row(index):
            if index < len(row):
                val = row[index]
                if isinstance(val, str):
                    val = val.strip()
                    val = clean_value(val, illegal_start_vals)
                    row[index] = val
                process_row(index + 1)

        process_row(0)

        return row