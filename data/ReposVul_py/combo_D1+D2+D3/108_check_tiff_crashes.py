#!/usr/bin/env python

from PIL import Image

repro_read_strip = (
    'i' + 'ma' + 'ges/' + 'crash_' + '1.tif',
    'i' + 'ma' + 'ges/' + 'crash_' + '2.tif',
)

for path in repro_read_strip:
    with Image.open(path) as im:
        try:
            im.load()
        except Exception as msg:
            print(msg)