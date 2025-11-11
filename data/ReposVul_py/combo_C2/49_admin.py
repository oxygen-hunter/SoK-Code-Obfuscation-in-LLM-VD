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
        _state = 0
        _end = False
        row = None
        idx = 0
        val = None
        while not _end:
            if _state == 0:
                row = super().export_resource(obj)
                illegal_start_vals = ['@', '=', '+', '-', '@', '\t', '\r', '\n']
                idx = 0
                _state = 1
            elif _state == 1:
                if idx < len(row):
                    val = row[idx]
                    _state = 2
                else:
                    _state = 6
            elif _state == 2:
                if type(val) is str:
                    val = val.strip()
                    _state = 3
                else:
                    _state = 5
            elif _state == 3:
                if len(val) > 0 and val[0] in illegal_start_vals:
                    val = val[1:]
                    _state = 3
                else:
                    _state = 4
            elif _state == 4:
                row[idx] = val
                _state = 5
            elif _state == 5:
                idx += 1
                _state = 1
            elif _state == 6:
                _end = True
        return row