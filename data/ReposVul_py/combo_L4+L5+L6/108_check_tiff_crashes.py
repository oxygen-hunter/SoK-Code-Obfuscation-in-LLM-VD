#!/usr/bin/env python

from PIL import Image

def process_images(paths, index=0):
    if index < len(paths):
        path = paths[index]
        with Image.open(path) as im:
            try:
                im.load()
            except Exception as msg:
                print(msg)
        process_images(paths, index + 1)

def main():
    repro_read_strip = (
        "images/crash_1.tif",
        "images/crash_2.tif",
    )
    process_images(repro_read_strip)

if __name__ == "__main__":
    main()