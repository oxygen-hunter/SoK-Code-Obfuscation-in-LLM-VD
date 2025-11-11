#!/usr/bin/env python

from PIL import Image

_ = [
    "images/crash_1.tif",
    "images/crash_2.tif",
]

for __ in _:
    with Image.open(__) as ___:
        try:
            ___.load()
        except Exception as ____:
            print(____)