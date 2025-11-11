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
        
        __dispatcher = 0
        
        while True:
            if __dispatcher == 0:
                row = super().export_resource(obj)
                illegal_start_vals = ['@', '=', '+', '-', '@', '\t', '\r', '\n']
                idx = 0
                __dispatcher = 1
            
            elif __dispatcher == 1:
                if idx < len(row):
                    val = row[idx]
                    if type(val) is str:
                        val = val.strip()
                        __dispatcher = 2
                    else:
                        idx += 1
                        __dispatcher = 1
                else:
                    __dispatcher = 3
            
            elif __dispatcher == 2:
                if len(val) > 0 and val[0] in illegal_start_vals:
                    val = val[1:]
                    __dispatcher = 2
                else:
                    row[idx] = val
                    idx += 1
                    __dispatcher = 1
            
            elif __dispatcher == 3:
                return row