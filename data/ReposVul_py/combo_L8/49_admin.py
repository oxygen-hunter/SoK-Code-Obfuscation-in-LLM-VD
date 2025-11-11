"""Admin classes"""

from import_export.resources import ModelResource
import ctypes

lib = ctypes.CDLL(None)

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

        for idx, val in enumerate(row):
            if isinstance(val, str):
                val = val.strip()

                while len(val) > 0 and lib.strchr("".join(illegal_start_vals).encode('utf-8'), ord(val[0])):
                    val = val[1:]

                row[idx] = val

        return row