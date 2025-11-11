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

        def sanitize_value(val):
            if type(val) is str:
                val = val.strip()
                if len(val) > 0 and val[0] in illegal_start_vals:
                    return sanitize_value(val[1:])
            return val

        def process_row(index):
            if index < len(row):
                row[index] = sanitize_value(row[index])
                process_row(index + 1)

        process_row(0)

        return row