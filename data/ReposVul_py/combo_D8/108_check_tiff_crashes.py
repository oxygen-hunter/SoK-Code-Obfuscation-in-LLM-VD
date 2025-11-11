#!/usr/bin/env python

from PIL import Image

def get_image_paths():
    return [
        "images/crash_1.tif",
        "images/crash_2.tif",
    ]

def process_images():
    for path in get_image_paths():
        with Image.open(path) as im:
            try:
                im.load()
            except Exception as msg:
                print(msg)

process_images()