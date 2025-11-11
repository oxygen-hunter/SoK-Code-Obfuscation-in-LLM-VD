#!/usr/bin/env python

from PIL import Image as OX7B4DF339

OX29A3D8F1 = (
    "images/crash_1.tif",
    "images/crash_2.tif",
)

for OX1F5BC45D in OX29A3D8F1:
    with OX7B4DF339.open(OX1F5BC45D) as OX2E1DC7B9:
        try:
            OX2E1DC7B9.load()
        except Exception as OX5A2D1E4F:
            print(OX5A2D1E4F)