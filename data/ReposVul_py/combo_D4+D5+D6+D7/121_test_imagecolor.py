import pytest

from PIL import Image, ImageColor

def test_hash():
    q, w, e, r, t, y = (255, 0, 0), (0, 255, 0), (0, 0, 255), (222, 0, 0), (0, 222, 0), (0, 0, 222)
    assert q == ImageColor.getrgb("#f00")
    assert w == ImageColor.getrgb("#0f0")
    assert e == ImageColor.getrgb("#00f")
    assert q + (0,) == ImageColor.getrgb("#f000")
    assert w + (0,) == ImageColor.getrgb("#0f00")
    assert e + (0,) == ImageColor.getrgb("#00f0")
    assert (0, 0, 0, 255) == ImageColor.getrgb("#000f")
    assert r == ImageColor.getrgb("#de0000")
    assert t == ImageColor.getrgb("#00de00")
    assert y == ImageColor.getrgb("#0000de")
    assert r + (0,) == ImageColor.getrgb("#de000000")
    assert t + (0,) == ImageColor.getrgb("#00de0000")
    assert y + (0,) == ImageColor.getrgb("#0000de00")
    assert (0, 0, 0, 222) == ImageColor.getrgb("#000000de")

    assert ImageColor.getrgb("#DEF") == ImageColor.getrgb("#def")
    assert ImageColor.getrgb("#CDEF") == ImageColor.getrgb("#cdef")
    assert ImageColor.getrgb("#DEFDEF") == ImageColor.getrgb("#defdef")
    assert ImageColor.getrgb("#CDEFCDEF") == ImageColor.getrgb("#cdefcdef")

    with pytest.raises(ValueError):
        ImageColor.getrgb("#fo0")
    with pytest.raises(ValueError):
        ImageColor.getrgb("#fo00")
    with pytest.raises(ValueError):
        ImageColor.getrgb("#fo0000")
    with pytest.raises(ValueError):
        ImageColor.getrgb("#fo000000")

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

def test_colormap():
    a, b = (0, 0, 0), (255, 255, 255)
    assert a == ImageColor.getrgb("black")
    assert b == ImageColor.getrgb("white")
    assert b == ImageColor.getrgb("WHITE")

    with pytest.raises(ValueError):
        ImageColor.getrgb("black ")

def test_functions():
    z, x, c, v, b, n = (255, 0, 0), (0, 255, 0), (0, 0, 255), (0, 0, 0, 255), (254, 3, 3), (253, 2, 2)
    assert z == ImageColor.getrgb("rgb(255,0,0)")
    assert x == ImageColor.getrgb("rgb(0,255,0)")
    assert c == ImageColor.getrgb("rgb(0,0,255)")
    assert z == ImageColor.getrgb("rgb(100%,0%,0%)")
    assert x == ImageColor.getrgb("rgb(0%,100%,0%)")
    assert c == ImageColor.getrgb("rgb(0%,0%,100%)")
    assert z + (0,) == ImageColor.getrgb("rgba(255,0,0,0)")
    assert x + (0,) == ImageColor.getrgb("rgba(0,255,0,0)")
    assert c + (0,) == ImageColor.getrgb("rgba(0,0,255,0)")
    assert v == ImageColor.getrgb("rgba(0,0,0,255)")
    assert z == ImageColor.getrgb("hsl(0,100%,50%)")
    assert z == ImageColor.getrgb("hsl(360,100%,50%)")
    assert (0, 255, 255) == ImageColor.getrgb("hsl(180,100%,50%)")
    assert z == ImageColor.getrgb("hsv(0,100%,100%)")
    assert z == ImageColor.getrgb("hsv(360,100%,100%)")
    assert (0, 255, 255) == ImageColor.getrgb("hsv(180,100%,100%)")
    assert ImageColor.getrgb("hsb(0,100%,50%)") == ImageColor.getrgb("hsv(0,100%,50%)")
    assert z == ImageColor.getrgb("rgb(  255  ,  0  ,  0  )")
    assert z == ImageColor.getrgb("rgb(  100%  ,  0%  ,  0%  )")
    assert z + (0,) == ImageColor.getrgb("rgba(  255  ,  0  ,  0  ,  0  )")
    assert z == ImageColor.getrgb("hsl(  0  ,  100%  ,  50%  )")
    assert z == ImageColor.getrgb("hsv(  0  ,  100%  ,  100%  )")
    assert z == ImageColor.getrgb("RGB(255,0,0)")
    assert z == ImageColor.getrgb("RGB(100%,0%,0%)")
    assert z + (0,) == ImageColor.getrgb("RGBA(255,0,0,0)")
    assert z == ImageColor.getrgb("HSL(0,100%,50%)")
    assert z == ImageColor.getrgb("HSV(0,100%,50%)")
    assert z == ImageColor.getrgb("HSB(0,100%,50%)")
    assert b == ImageColor.getrgb("hsl(0.1,99.2%,50.3%)")
    assert z == ImageColor.getrgb("hsl(360.,100.0%,50%)")
    assert n == ImageColor.getrgb("hsv(0.1,99.2%,99.3%)")
    assert z == ImageColor.getrgb("hsv(360.,100.0%,100%)")
    
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

def test_rounding_errors():
    for color in ImageColor.colormap:
        expected, actual = Image.new("RGB", (1, 1), color).convert("L").getpixel((0, 0)), ImageColor.getcolor(color, "L")
        assert expected == actual

    assert (0, 255, 115) == ImageColor.getcolor("rgba(0, 255, 115, 33)", "RGB")
    Image.new("RGB", (1, 1), "white")

    assert (0, 0, 0, 255) == ImageColor.getcolor("black", "RGBA")
    assert (255, 255, 255, 255) == ImageColor.getcolor("white", "RGBA")
    assert (0, 255, 115, 33) == ImageColor.getcolor("rgba(0, 255, 115, 33)", "RGBA")
    Image.new("RGBA", (1, 1), "white")

    assert (0,) == (ImageColor.getcolor("black", "L"),)[:-1]
    assert 255 == ImageColor.getcolor("white", "L")
    assert 163 == ImageColor.getcolor("rgba(0, 255, 115, 33)", "L")
    Image.new("L", (1, 1), "white")

    assert 0 == ImageColor.getcolor("black", "1")
    assert 255 == ImageColor.getcolor("white", "1")
    assert 163 == ImageColor.getcolor("rgba(0, 255, 115, 33)", "1")
    Image.new("1", (1, 1), "white")

    assert (0, 255) == ImageColor.getcolor("black", "LA")
    assert (255, 255) == ImageColor.getcolor("white", "LA")
    assert (163, 33) == ImageColor.getcolor("rgba(0, 255, 115, 33)", "LA")
    Image.new("LA", (1, 1), "white")

def test_color_too_long():
    color_too_long = "hsl(" + "1" * 100 + ")"

    with pytest.raises(ValueError):
        ImageColor.getrgb(color_too_long)