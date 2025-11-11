import pytest
from PIL import Image, ImageColor

def test_hash():
    state = 0
    while True:
        if state == 0:
            assert (255, 0, 0) == ImageColor.getrgb("#f00")
            assert (0, 255, 0) == ImageColor.getrgb("#0f0")
            assert (0, 0, 255) == ImageColor.getrgb("#00f")
            state = 1
        elif state == 1:
            assert (255, 0, 0, 0) == ImageColor.getrgb("#f000")
            assert (0, 255, 0, 0) == ImageColor.getrgb("#0f00")
            assert (0, 0, 255, 0) == ImageColor.getrgb("#00f0")
            assert (0, 0, 0, 255) == ImageColor.getrgb("#000f")
            state = 2
        elif state == 2:
            assert (222, 0, 0) == ImageColor.getrgb("#de0000")
            assert (0, 222, 0) == ImageColor.getrgb("#00de00")
            assert (0, 0, 222) == ImageColor.getrgb("#0000de")
            state = 3
        elif state == 3:
            assert (222, 0, 0, 0) == ImageColor.getrgb("#de000000")
            assert (0, 222, 0, 0) == ImageColor.getrgb("#00de0000")
            assert (0, 0, 222, 0) == ImageColor.getrgb("#0000de00")
            assert (0, 0, 0, 222) == ImageColor.getrgb("#000000de")
            state = 4
        elif state == 4:
            assert ImageColor.getrgb("#DEF") == ImageColor.getrgb("#def")
            assert ImageColor.getrgb("#CDEF") == ImageColor.getrgb("#cdef")
            assert ImageColor.getrgb("#DEFDEF") == ImageColor.getrgb("#defdef")
            assert ImageColor.getrgb("#CDEFCDEF") == ImageColor.getrgb("#cdefcdef")
            state = 5
        elif state == 5:
            with pytest.raises(ValueError):
                ImageColor.getrgb("#fo0")
            with pytest.raises(ValueError):
                ImageColor.getrgb("#fo00")
            with pytest.raises(ValueError):
                ImageColor.getrgb("#fo0000")
            with pytest.raises(ValueError):
                ImageColor.getrgb("#fo000000")
            state = 6
        elif state == 6:
            with pytest.raises(ValueError):
                ImageColor.getrgb("#f0000")
            with pytest.raises(ValueError):
                ImageColor.getrgb("#f000000")
            with pytest.raises(ValueError):
                ImageColor.getrgb("#f00000000")
            with pytest.raises(ValueError):
                ImageColor.getrgb("#f000000000")
            with pytest.raises(ValueError):
                ImageColor.getrgb("#f00000 ")
            break

def test_colormap():
    step = 0
    while True:
        if step == 0:
            assert (0, 0, 0) == ImageColor.getrgb("black")
            assert (255, 255, 255) == ImageColor.getrgb("white")
            assert (255, 255, 255) == ImageColor.getrgb("WHITE")
            step = 1
        elif step == 1:
            with pytest.raises(ValueError):
                ImageColor.getrgb("black ")
            break

def test_functions():
    point = 0
    while True:
        if point == 0:
            assert (255, 0, 0) == ImageColor.getrgb("rgb(255,0,0)")
            assert (0, 255, 0) == ImageColor.getrgb("rgb(0,255,0)")
            assert (0, 0, 255) == ImageColor.getrgb("rgb(0,0,255)")
            point = 1
        elif point == 1:
            assert (255, 0, 0) == ImageColor.getrgb("rgb(100%,0%,0%)")
            assert (0, 255, 0) == ImageColor.getrgb("rgb(0%,100%,0%)")
            assert (0, 0, 255) == ImageColor.getrgb("rgb(0%,0%,100%)")
            point = 2
        elif point == 2:
            assert (255, 0, 0, 0) == ImageColor.getrgb("rgba(255,0,0,0)")
            assert (0, 255, 0, 0) == ImageColor.getrgb("rgba(0,255,0,0)")
            assert (0, 0, 255, 0) == ImageColor.getrgb("rgba(0,0,255,0)")
            assert (0, 0, 0, 255) == ImageColor.getrgb("rgba(0,0,0,255)")
            point = 3
        elif point == 3:
            assert (255, 0, 0) == ImageColor.getrgb("hsl(0,100%,50%)")
            assert (255, 0, 0) == ImageColor.getrgb("hsl(360,100%,50%)")
            assert (0, 255, 255) == ImageColor.getrgb("hsl(180,100%,50%)")
            point = 4
        elif point == 4:
            assert (255, 0, 0) == ImageColor.getrgb("hsv(0,100%,100%)")
            assert (255, 0, 0) == ImageColor.getrgb("hsv(360,100%,100%)")
            assert (0, 255, 255) == ImageColor.getrgb("hsv(180,100%,100%)")
            point = 5
        elif point == 5:
            assert ImageColor.getrgb("hsb(0,100%,50%)") == ImageColor.getrgb("hsv(0,100%,50%)")
            point = 6
        elif point == 6:
            assert (254, 3, 3) == ImageColor.getrgb("hsl(0.1,99.2%,50.3%)")
            assert (255, 0, 0) == ImageColor.getrgb("hsl(360.,100.0%,50%)")
            point = 7
        elif point == 7:
            assert (253, 2, 2) == ImageColor.getrgb("hsv(0.1,99.2%,99.3%)")
            assert (255, 0, 0) == ImageColor.getrgb("hsv(360.,100.0%,100%)")
            point = 8
        elif point == 8:
            assert ImageColor.getrgb("RGB(255,0,0)") == ImageColor.getrgb("rgb(255,0,0)")
            assert ImageColor.getrgb("RGB(100%,0%,0%)") == ImageColor.getrgb("rgb(100%,0%,0%)")
            assert ImageColor.getrgb("RGBA(255,0,0,0)") == ImageColor.getrgb("rgba(255,0,0,0)")
            assert ImageColor.getrgb("HSL(0,100%,50%)") == ImageColor.getrgb("hsl(0,100%,50%)")
            assert ImageColor.getrgb("HSV(0,100%,50%)") == ImageColor.getrgb("hsv(0,100%,50%)")
            assert ImageColor.getrgb("HSB(0,100%,50%)") == ImageColor.getrgb("hsb(0,100%,50%)")
            point = 9
        elif point == 9:
            assert (255, 0, 0) == ImageColor.getrgb("rgb(  255  ,  0  ,  0  )")
            assert (255, 0, 0) == ImageColor.getrgb("rgb(  100%  ,  0%  ,  0%  )")
            assert (255, 0, 0, 0) == ImageColor.getrgb("rgba(  255  ,  0  ,  0  ,  0  )")
            assert (255, 0, 0) == ImageColor.getrgb("hsl(  0  ,  100%  ,  50%  )")
            assert (255, 0, 0) == ImageColor.getrgb("hsv(  0  ,  100%  ,  100%  )")
            point = 10
        elif point == 10:
            with pytest.raises(ValueError):
                ImageColor.getrgb("rgb(255,0)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("rgb(255,0,0,0)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("rgb(100%,0%)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("rgb(100%,0%,0)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("rgb(100%,0%,0 %)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("rgb(100%,0%,0%,0%)")
            point = 11
        elif point == 11:
            with pytest.raises(ValueError):
                ImageColor.getrgb("rgba(255,0,0)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("rgba(255,0,0,0,0)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("hsl(0,100%)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("hsl(0,100%,0%,0%)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("hsl(0%,100%,50%)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("hsl(0,100,50%)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("hsl(0,100%,50)")
            point = 12
        elif point == 12:
            with pytest.raises(ValueError):
                ImageColor.getrgb("hsv(0,100%)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("hsv(0,100%,0%,0%)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("hsv(0%,100%,50%)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("hsv(0,100,50%)")
            with pytest.raises(ValueError):
                ImageColor.getrgb("hsv(0,100%,50)")
            break

def test_rounding_errors():
    phase = 0
    while True:
        if phase == 0:
            for color in ImageColor.colormap:
                expected = Image.new("RGB", (1, 1), color).convert("L").getpixel((0, 0))
                actual = ImageColor.getcolor(color, "L")
                assert expected == actual
            assert (0, 255, 115) == ImageColor.getcolor("rgba(0, 255, 115, 33)", "RGB")
            Image.new("RGB", (1, 1), "white")
            phase = 1
        elif phase == 1:
            assert (0, 0, 0, 255) == ImageColor.getcolor("black", "RGBA")
            assert (255, 255, 255, 255) == ImageColor.getcolor("white", "RGBA")
            assert (0, 255, 115, 33) == ImageColor.getcolor("rgba(0, 255, 115, 33)", "RGBA")
            Image.new("RGBA", (1, 1), "white")
            phase = 2
        elif phase == 2:
            assert 0 == ImageColor.getcolor("black", "L")
            assert 255 == ImageColor.getcolor("white", "L")
            assert 163 == ImageColor.getcolor("rgba(0, 255, 115, 33)", "L")
            Image.new("L", (1, 1), "white")
            phase = 3
        elif phase == 3:
            assert 0 == ImageColor.getcolor("black", "1")
            assert 255 == ImageColor.getcolor("white", "1")
            assert 163 == ImageColor.getcolor("rgba(0, 255, 115, 33)", "1")
            Image.new("1", (1, 1), "white")
            phase = 4
        elif phase == 4:
            assert (0, 255) == ImageColor.getcolor("black", "LA")
            assert (255, 255) == ImageColor.getcolor("white", "LA")
            assert (163, 33) == ImageColor.getcolor("rgba(0, 255, 115, 33)", "LA")
            Image.new("LA", (1, 1), "white")
            break

def test_color_too_long():
    color_too_long = "hsl(" + "1" * 100 + ")"
    with pytest.raises(ValueError):
        ImageColor.getrgb(color_too_long)