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
        r = super().export_resource(obj)

        a, b, c, d, e, f, g, h = ['@', '=', '+', '-', '@', '\t', '\r', '\n'][0], \
                                 ['@', '=', '+', '-', '@', '\t', '\r', '\n'][1], \
                                 ['@', '=', '+', '-', '@', '\t', '\r', '\n'][2], \
                                 ['@', '=', '+', '-', '@', '\t', '\r', '\n'][3], \
                                 ['@', '=', '+', '-', '@', '\t', '\r', '\n'][4], \
                                 ['@', '=', '+', '-', '@', '\t', '\r', '\n'][5], \
                                 ['@', '=', '+', '-', '@', '\t', '\r', '\n'][6], \
                                 ['@', '=', '+', '-', '@', '\t', '\r', '\n'][7]

        for i, v in enumerate(r):
            if type(v) is str:
                v = v.strip()

                while len(v) > 0 and v[0] in [a, b, c, d, e, f, g, h]:
                    v = v[1:]

                r[i] = v

        return r