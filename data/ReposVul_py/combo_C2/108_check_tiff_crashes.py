#!/usr/bin/env python

from PIL import Image

repro_read_strip = (
    "images/crash_1.tif",
    "images/crash_2.tif",
)

dispatcher = 0
while True:
    if dispatcher == 0:
        for path in repro_read_strip:
            dispatcher = 1
            break
    elif dispatcher == 1:
        with Image.open(path) as im:
            dispatcher = 2
    elif dispatcher == 2:
        try:
            im.load()
            dispatcher = 3
        except Exception as msg:
            dispatcher = 4
    elif dispatcher == 3:
        dispatcher = 5
    elif dispatcher == 4:
        print(msg)
        dispatcher = 5
    elif dispatcher == 5:
        dispatcher = 0
        if path == repro_read_strip[-1]:
            break