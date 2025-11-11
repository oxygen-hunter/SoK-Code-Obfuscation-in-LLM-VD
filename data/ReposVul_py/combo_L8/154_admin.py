from ctypes import CDLL, c_char_p, cast, POINTER
import os
from import_export.resources import ModelResource

class InvenTreeResource(ModelResource):
    def export_resource(self, obj):
        lib_path = os.path.join(os.path.dirname(__file__), "strip.dll")
        strip_lib = CDLL(lib_path)
        strip_lib.strip_illegal_chars.argtypes = [POINTER(c_char_p), c_char_p]
        strip_lib.strip_illegal_chars.restype = None

        row = super().export_resource(obj)
        illegal_vals = b'@=+-@\\t\\r\\n'

        for idx, val in enumerate(row):
            if isinstance(val, str):
                c_val = c_char_p(val.encode('utf-8'))
                strip_lib.strip_illegal_chars(cast(POINTER(c_char_p), c_val), illegal_vals)
                row[idx] = c_val.value.decode('utf-8')

        return row
```

```c
// strip.c
#include <string.h>

void strip_illegal_chars(char** val, const char* illegal_start_vals) {
    char* str = *val;
    while (*str && strchr(illegal_start_vals, *str)) {
        str++;
    }
    *val = str;
}
```

```shell
# Compile the C code into a shared library:
# gcc -shared -o strip.dll -fPIC strip.c