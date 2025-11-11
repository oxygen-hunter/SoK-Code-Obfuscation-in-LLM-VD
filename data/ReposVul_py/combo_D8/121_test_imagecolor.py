import pytest

from PIL import Image, ImageColor


def test_hash():
    def v1(): return (255, 0, 0)
    def v2(): return (0, 255, 0)
    def v3(): return (0, 0, 255)
    def v4(): return (255, 0, 0, 0)
    def v5(): return (0, 255, 0, 0)
    def v6(): return (0, 0, 255, 0)
    def v7(): return (0, 0, 0, 255)
    def v8(): return (222, 0, 0)
    def v9(): return (0, 222, 0)
    def v10(): return (0, 0, 222)
    def v11(): return (222, 0, 0, 0)
    def v12(): return (0, 222, 0, 0)
    def v13(): return (0, 0, 222, 0)
    def v14(): return (0, 0, 0, 222)

    assert v1() == ImageColor.getrgb("#f00")
    assert v2() == ImageColor.getrgb("#0f0")
    assert v3() == ImageColor.getrgb("#00f")

    assert v4() == ImageColor.getrgb("#f000")
    assert v5() == ImageColor.getrgb("#0f00")
    assert v6() == ImageColor.getrgb("#00f0")
    assert v7() == ImageColor.getrgb("#000f")

    assert v8() == ImageColor.getrgb("#de0000")
    assert v9() == ImageColor.getrgb("#00de00")
    assert v10() == ImageColor.getrgb("#0000de")

    assert v11() == ImageColor.getrgb("#de000000")
    assert v12() == ImageColor.getrgb("#00de0000")
    assert v13() == ImageColor.getrgb("#0000de00")
    assert v14() == ImageColor.getrgb("#000000de")

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
    def v1(): return (0, 0, 0)
    def v2(): return (255, 255, 255)
    
    assert v1() == ImageColor.getrgb("black")
    assert v2() == ImageColor.getrgb("white")
    assert v2() == ImageColor.getrgb("WHITE")

    with pytest.raises(ValueError):
        ImageColor.getrgb("black ")


def test_functions():
    def v1(): return (255, 0, 0)
    def v2(): return (0, 255, 0)
    def v3(): return (0, 0, 255)
    def v4(): return (255, 0, 0, 0)
    def v5(): return (0, 0, 0, 255)
    def v6(): return (0, 255, 255)
    def v7(): return (254, 3, 3)
    def v8(): return (253, 2, 2)

    assert v1() == ImageColor.getrgb("rgb(255,0,0)")
    assert v2() == ImageColor.getrgb("rgb(0,255,0)")
    assert v3() == ImageColor.getrgb("rgb(0,0,255)")

    assert v1() == ImageColor.getrgb("rgb(100%,0%,0%)")
    assert v2() == ImageColor.getrgb("rgb(0%,100%,0%)")
    assert v3() == ImageColor.getrgb("rgb(0%,0%,100%)")

    assert v4() == ImageColor.getrgb("rgba(255,0,0,0)")
    assert v4() == ImageColor.getrgb("rgba(0,255,0,0)")
    assert v4() == ImageColor.getrgb("rgba(0,0,255,0)")
    assert v5() == ImageColor.getrgb("rgba(0,0,0,255)")

    assert v1() == ImageColor.getrgb("hsl(0,100%,50%)")
    assert v1() == ImageColor.getrgb("hsl(360,100%,50%)")
    assert v6() == ImageColor.getrgb("hsl(180,100%,50%)")

    assert v1() == ImageColor.getrgb("hsv(0,100%,100%)")
    assert v1() == ImageColor.getrgb("hsv(360,100%,100%)")
    assert v6() == ImageColor.getrgb("hsv(180,100%,100%)")

    assert ImageColor.getrgb("hsb(0,100%,50%)") == ImageColor.getrgb("hsv(0,100%,50%)")

    assert v7() == ImageColor.getrgb("hsl(0.1,99.2%,50.3%)")
    assert v1() == ImageColor.getrgb("hsl(360.,100.0%,50%)")

    assert v8() == ImageColor.getrgb("hsv(0.1,99.2%,99.3%)")
    assert v1() == ImageColor.getrgb("hsv(360.,100.0%,100%)")

    assert ImageColor.getrgb("RGB(255,0,0)") == ImageColor.getrgb("rgb(255,0,0)")
    assert ImageColor.getrgb("RGB(100%,0%,0%)") == ImageColor.getrgb("rgb(100%,0%,0%)")
    assert ImageColor.getrgb("RGBA(255,0,0,0)") == ImageColor.getrgb("rgba(255,0,0,0)")
    assert ImageColor.getrgb("HSL(0,100%,50%)") == ImageColor.getrgb("hsl(0,100%,50%)")
    assert ImageColor.getrgb("HSV(0,100%,50%)") == ImageColor.getrgb("hsv(0,100%,50%)")
    assert ImageColor.getrgb("HSB(0,100%,50%)") == ImageColor.getrgb("hsb(0,100%,50%)")

    assert v1() == ImageColor.getrgb("rgb(  255  ,  0  ,  0  )")
    assert v1() == ImageColor.getrgb("rgb(  100%  ,  0%  ,  0%  )")
    assert v4() == ImageColor.getrgb("rgba(  255  ,  0  ,  0  ,  0  )")
    assert v1() == ImageColor.getrgb("hsl(  0  ,  100%  ,  50%  )")
    assert v1() == ImageColor.getrgb("hsv(  0  ,  100%  ,  100%  )")

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
        expected = Image.new("RGB", (1, 1), color).convert("L").getpixel((0, 0))
        actual = ImageColor.getcolor(color, "L")
        assert expected == actual

    def v1(): return (0, 255, 115)
    def v2(): return (0, 0, 0, 255)
    def v3(): return (255, 255, 255, 255)
    def v4(): return (0, 255, 115, 33)
    def v5(): return 0
    def v6(): return 255
    def v7(): return 163
    def v8(): return (0, 255)
    def v9(): return (255, 255)

    assert v1() == ImageColor.getcolor("rgba(0, 255, 115, 33)", "RGB")
    Image.new("RGB", (1, 1), "white")

    assert v2() == ImageColor.getcolor("black", "RGBA")
    assert v3() == ImageColor.getcolor("white", "RGBA")
    assert v4() == ImageColor.getcolor("rgba(0, 255, 115, 33)", "RGBA")
    Image.new("RGBA", (1, 1), "white")

    assert v5() == ImageColor.getcolor("black", "L")
    assert v6() == ImageColor.getcolor("white", "L")
    assert v7() == ImageColor.getcolor("rgba(0, 255, 115, 33)", "L")
    Image.new("L", (1, 1), "white")

    assert v5() == ImageColor.getcolor("black", "1")
    assert v6() == ImageColor.getcolor("white", "1")
    assert v7() == ImageColor.getcolor("rgba(0, 255, 115, 33)", "1")
    Image.new("1", (1, 1), "white")

    assert v8() == ImageColor.getcolor("black", "LA")
    assert v9() == ImageColor.getcolor("white", "LA")
    assert v4() == ImageColor.getcolor("rgba(0, 255, 115, 33)", "LA")
    Image.new("LA", (1, 1), "white")


def test_color_too_long():
    color_too_long = "hsl(" + "1" * 100 + ")"

    with pytest.raises(ValueError):
        ImageColor.getrgb(color_too_long)