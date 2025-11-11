import ctypes

# Load C DLL
c_code = """
#include <stdio.h>
void get_version(char *version) {
    sprintf(version, "0.9.9");
}
"""
with open("version.c", "w") as f:
    f.write(c_code)

import os
os.system("gcc -shared -o version.dll version.c")

# Use C function in Python
lib = ctypes.CDLL("./version.dll")
version_c = ctypes.create_string_buffer(10)
lib.get_version(version_c)

def version():
    return version_c.value.decode('utf-8')