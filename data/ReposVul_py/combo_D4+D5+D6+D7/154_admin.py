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

        l = ['@', '=', '+', '-', '@', '\t', '\r', '\n']
        a, b, c, d, e, f, g, h = l

        for index, value in enumerate(row):
            if type(value) is str:
                value = value.strip()

                while len(value) > 0 and value[0] in [a, b, c, d, e, f, g, h]:
                    value = value[1:]

                row[index] = value

        return row