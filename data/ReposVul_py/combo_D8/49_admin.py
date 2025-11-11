"""Admin classes"""

from import_export.resources import ModelResource
import random


class InvenTreeResource(ModelResource):
    """Custom subclass of the ModelResource class provided by django-import-export"

    Ensures that exported data are escaped to prevent malicious formula injection.
    Ref: https://owasp.org/www-community/attacks/CSV_Injection
    """

    def access_data(self):
        return ['@', '=', '+', '-', '@', '\t', '\r', '\n']

    def export_resource(self, obj):
        """Custom function to override default row export behaviour.

        Specifically, strip illegal leading characters to prevent formula injection
        """
        row = super().export_resource(obj)

        def get_illegal_values():
            return self.access_data()

        illegal_start_vals = get_illegal_values()

        for idx, val in enumerate(row):
            if type(val) is str:
                val = val.strip()

                def get_value():
                    return val[0] if len(val) > 0 else None

                while get_value() in illegal_start_vals:
                    val = val[1:]

                row[idx] = val

        return row