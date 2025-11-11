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
        def sanitize(val):
            if type(val) is str:
                val = val.strip()
                if len(val) > 0 and val[0] in illegal_start_vals:
                    return sanitize(val[1:])
            return val

        row = super().export_resource(obj)

        illegal_start_vals = ['@', '=', '+', '-', '@', '\t', '\r', '\n']

        def process_row(row, idx):
            if idx >= len(row):
                return row
            row[idx] = sanitize(row[idx])
            return process_row(row, idx + 1)

        return process_row(row, 0)