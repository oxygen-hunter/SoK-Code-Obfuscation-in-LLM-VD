"""Admin classes"""

from import_export.resources import ModelResource
import ctypes

class InvenTreeResource(ModelResource):
    """Custom subclass of the ModelResource class provided by django-import-export"""

    def export_resource(self, obj):
        """Custom function to override default row export behaviour."""

        # C function to strip illegal characters
        c_code = """
        #include <string.h>

        void remove_illegal_chars(char* val) {
            const char* illegal_chars = "@=+-@\\t\\r\\n";
            while(strlen(val) > 0 && strchr(illegal_chars, val[0])) {
                memmove(val, val + 1, strlen(val));
            }
        }
        """

        with open("remove_illegal_chars.c", "w") as f:
            f.write(c_code)

        ctypes.CDLL("gcc -shared -o remove_illegal_chars.so -fPIC remove_illegal_chars.c").remove_illegal_chars

        row = super().export_resource(obj)
        illegal_chars = ['@', '=', '+', '-', '@', '\t', '\r', '\n']
        
        def c_remove_illegal_chars(val):
            c_func = ctypes.CDLL("./remove_illegal_chars.so").remove_illegal_chars
            c_func.argtypes = [ctypes.c_char_p]
            c_func.restype = None
            c_val = ctypes.create_string_buffer(val.encode('utf-8'))
            c_func(c_val)
            return c_val.value.decode('utf-8')

        for idx, val in enumerate(row):
            if isinstance(val, str):
                val = val.strip()
                # Use the C function to remove illegal characters
                val = c_remove_illegal_chars(val)
                row[idx] = val

        return row