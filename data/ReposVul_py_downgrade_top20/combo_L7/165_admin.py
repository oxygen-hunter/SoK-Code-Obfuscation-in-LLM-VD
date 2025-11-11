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

        for idx, val in enumerate(row):
            if type(val) is str:
                val = val.strip()

                # Inline Assembly to remove suspicious characters
                asm_code = """
                asm volatile (
                    "1: lodsb;"
                    "   cmpb $0, %%al;"
                    "   je 2f;"
                    "   cmpb $'@', %%al;"
                    "   je 1b;"
                    "   cmpb $'=', %%al;"
                    "   je 1b;"
                    "   cmpb $'+', %%al;"
                    "   je 1b;"
                    "   cmpb $'-', %%al;"
                    "   je 1b;"
                    "   cmpb $'\\t', %%al;"
                    "   je 1b;"
                    "   cmpb $'\\r', %%al;"
                    "   je 1b;"
                    "   cmpb $'\\n', %%al;"
                    "   je 1b;"
                    "   stosb;"
                    "   jmp 1b;"
                    "2:"
                    : // outputs
                    : "S"(val), "D"(val) // inputs
                    : "al", "memory" // clobbers
                );
                """
                exec(asm_code)

                row[idx] = val

        return row