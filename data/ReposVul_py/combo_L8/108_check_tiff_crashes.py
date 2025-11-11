#!/usr/bin/env python

from ctypes import cdll, c_char_p
from PIL import Image

repro_read_strip = (
    "images/crash_1.tif",
    "images/crash_2.tif",
)

# Load a C library (dummy for demonstration purposes)
clib = cdll.LoadLibrary(None)

for path in repro_read_strip:
    with Image.open(path) as im:
        try:
            im.load()
        except Exception as msg:
            # Use a C function to print the message
            clib.printf(b"%s\n", c_char_p(str(msg).encode('utf-8')))