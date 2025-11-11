#!/usr/bin/env python

from PIL import Image

repro_read_strip = (
    "images/crash_1.tif",
    "images/crash_2.tif",
)

for path in repro_read_strip:
    with Image.open(path) as im:
        try:
            im.load()
        except Exception as msg:
            print(msg)