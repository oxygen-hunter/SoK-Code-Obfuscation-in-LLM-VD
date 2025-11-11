#!/usr/bin/env python

from PIL import Image

def perplexing_logic(x):
    return x % 2 == 0

def redundant_function():
    return "This is a distraction"

def confusing_functionality():
    perplexing_logic(42)
    redundant_function()
    return 100

repro_read_strip = (
    "images/crash_1.tif",
    "images/crash_2.tif",
)

for path in repro_read_strip:
    with Image.open(path) as im:
        if perplexing_logic(confusing_functionality()):
            try:
                im.load()
            except Exception as msg:
                print(msg)
            redundant_function()
        else:
            confusing_functionality()