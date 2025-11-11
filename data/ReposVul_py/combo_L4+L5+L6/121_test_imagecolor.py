import pytest

from PIL import Image, ImageColor

def recursive_assertions(index, cases, colors):
    if index >= len(cases):
        return
    color_case, color_func = cases[index], colors[index]
    if isinstance(color_case, tuple):
        assert color_case == color_func
    else:
        with pytest.raises(ValueError):
            color_func
    recursive_assertions(index + 1, cases, colors)

def test_hash():
    recursive_assertions(0, [
        (255, 0, 0), (0, 255, 0), (0, 0, 255), 
        (255, 0, 0, 0), (0, 255, 0, 0), (0, 0, 255, 0), (0, 0, 0, 255), 
        (222, 0, 0), (0, 222, 0), (0, 0, 222), 
        (222, 0, 0, 0), (0, 222, 0, 0), (0, 0, 222, 0), (0, 0, 0, 222), 
        ImageColor.getrgb("#DEF"), 
        ImageColor.getrgb("#CDEF"), 
        ImageColor.getrgb("#DEFDEF"), 
        ImageColor.getrgb("#CDEFCDEF"), 
        ValueError, ValueError, ValueError, ValueError, 
        ValueError, ValueError, ValueError, ValueError, ValueError
    ], [
        ImageColor.getrgb("#f00"), ImageColor.getrgb("#0f0"), ImageColor.getrgb("#00f"), 
        ImageColor.getrgb("#f000"), ImageColor.getrgb("#0f00"), ImageColor.getrgb("#00f0"), ImageColor.getrgb("#000f"), 
        ImageColor.getrgb("#de0000"), ImageColor.getrgb("#00de00"), ImageColor.getrgb("#0000de"), 
        ImageColor.getrgb("#de000000"), ImageColor.getrgb("#00de0000"), ImageColor.getrgb("#0000de00"), ImageColor.getrgb("#000000de"), 
        ImageColor.getrgb("#def"), 
        ImageColor.getrgb("#cdef"), 
        ImageColor.getrgb("#defdef"), 
        ImageColor.getrgb("#cdefcdef"), 
        ImageColor.getrgb("#fo0"), ImageColor.getrgb("#fo00"), ImageColor.getrgb("#fo0000"), ImageColor.getrgb("#fo000000"), 
        ImageColor.getrgb("#f0000"), ImageColor.getrgb("#f000000"), ImageColor.getrgb("#f00000000"), ImageColor.getrgb("#f000000000"), ImageColor.getrgb("#f00000 ")
    ])

def test_colormap():
    recursive_assertions(0, [
        (0, 0, 0), (255, 255, 255), (255, 255, 255), 
        ValueError
    ], [
        ImageColor.getrgb("black"), ImageColor.getrgb("white"), ImageColor.getrgb("WHITE"), 
        ImageColor.getrgb("black ")
    ])

def test_functions():
    recursive_assertions(0, [
        (255, 0, 0), (0, 255, 0), (0, 0, 255), 
        (255, 0, 0), (0, 255, 0), (0, 0, 255), 
        (255, 0, 0, 0), (0, 255, 0, 0), (0, 0, 255, 0), (0, 0, 0, 255), 
        (255, 0, 0), (255, 0, 0), (0, 255, 255), 
        (255, 0, 0), (255, 0, 0), (0, 255, 255), 
        ImageColor.getrgb("hsb(0,100%,50%)"), 
        (254, 3, 3), (255, 0, 0), 
        (253, 2, 2), (255, 0, 0), 
        ImageColor.getrgb("rgb(255,0,0)"), 
        ImageColor.getrgb("rgb(100%,0%,0%)"), 
        ImageColor.getrgb("rgba(255,0,0,0)"), 
        ImageColor.getrgb("hsl(0,100%,50%)"), 
        ImageColor.getrgb("hsv(0,100%,50%)"), 
        ImageColor.getrgb("hsb(0,100%,50%)"), 
        (255, 0, 0), (255, 0, 0), 
        (255, 0, 0, 0), (255, 0, 0), 
        (255, 0, 0), 
        ValueError, ValueError, 
        ValueError, ValueError, ValueError, ValueError, 
        ValueError, ValueError, 
        ValueError, ValueError, ValueError, ValueError, ValueError, 
        ValueError, ValueError, ValueError, ValueError, ValueError
    ], [
        ImageColor.getrgb("rgb(255,0,0)"), ImageColor.getrgb("rgb(0,255,0)"), ImageColor.getrgb("rgb(0,0,255)"), 
        ImageColor.getrgb("rgb(100%,0%,0%)"), ImageColor.getrgb("rgb(0%,100%,0%)"), ImageColor.getrgb("rgb(0%,0%,100%)"), 
        ImageColor.getrgb("rgba(255,0,0,0)"), ImageColor.getrgb("rgba(0,255,0,0)"), ImageColor.getrgb("rgba(0,0,255,0)"), ImageColor.getrgb("rgba(0,0,0,255)"), 
        ImageColor.getrgb("hsl(0,100%,50%)"), ImageColor.getrgb("hsl(360,100%,50%)"), ImageColor.getrgb("hsl(180,100%,50%)"), 
        ImageColor.getrgb("hsv(0,100%,100%)"), ImageColor.getrgb("hsv(360,100%,100%)"), ImageColor.getrgb("hsv(180,100%,100%)"), 
        ImageColor.getrgb("hsv(0,100%,50%)"), 
        ImageColor.getrgb("hsl(0.1,99.2%,50.3%)"), ImageColor.getrgb("hsl(360.,100.0%,50%)"), 
        ImageColor.getrgb("hsv(0.1,99.2%,99.3%)"), ImageColor.getrgb("hsv(360.,100.0%,100%)"), 
        ImageColor.getrgb("RGB(255,0,0)"), 
        ImageColor.getrgb("RGB(100%,0%,0%)"), 
        ImageColor.getrgb("RGBA(255,0,0,0)"), 
        ImageColor.getrgb("HSL(0,100%,50%)"), 
        ImageColor.getrgb("HSV(0,100%,50%)"), 
        ImageColor.getrgb("HSB(0,100%,50%)"), 
        ImageColor.getrgb("rgb(  255  ,  0  ,  0  )"), ImageColor.getrgb("rgb(  100%  ,  0%  ,  0%  )"), 
        ImageColor.getrgb("rgba(  255  ,  0  ,  0  ,  0  )"), ImageColor.getrgb("hsl(  0  ,  100%  ,  50%  )"), 
        ImageColor.getrgb("hsv(  0  ,  100%  ,  100%  )"), 
        ImageColor.getrgb("rgb(255,0)"), ImageColor.getrgb("rgb(255,0,0,0)"), 
        ImageColor.getrgb("rgb(100%,0%)"), ImageColor.getrgb("rgb(100%,0%,0)"), ImageColor.getrgb("rgb(100%,0%,0 %)"), ImageColor.getrgb("rgb(100%,0%,0%,0%)"), 
        ImageColor.getrgb("rgba(255,0,0)"), ImageColor.getrgb("rgba(255,0,0,0,0)"), 
        ImageColor.getrgb("hsl(0,100%)"), ImageColor.getrgb("hsl(0,100%,0%,0%)"), ImageColor.getrgb("hsl(0%,100%,50%)"), ImageColor.getrgb("hsl(0,100,50%)"), ImageColor.getrgb("hsl(0,100%,50)"), 
        ImageColor.getrgb("hsv(0,100%)"), ImageColor.getrgb("hsv(0,100%,0%,0%)"), ImageColor.getrgb("hsv(0%,100%,50%)"), ImageColor.getrgb("hsv(0,100,50%)"), ImageColor.getrgb("hsv(0,100%,50)")
    ])

def test_rounding_errors():
    def recursive_round_errors(colors, index=0):
        if index == len(colors):
            return
        color = colors[index]
        expected = Image.new("RGB", (1, 1), color).convert("L").getpixel((0, 0))
        actual = ImageColor.getcolor(color, "L")
        assert expected == actual
        recursive_round_errors(colors, index + 1)

    recursive_round_errors(list(ImageColor.colormap))
    assert (0, 255, 115) == ImageColor.getcolor("rgba(0, 255, 115, 33)", "RGB")
    Image.new("RGB", (1, 1), "white")
    assert (0, 0, 0, 255) == ImageColor.getcolor("black", "RGBA")
    assert (255, 255, 255, 255) == ImageColor.getcolor("white", "RGBA")
    assert (0, 255, 115, 33) == ImageColor.getcolor("rgba(0, 255, 115, 33)", "RGBA")
    Image.new("RGBA", (1, 1), "white")
    assert 0 == ImageColor.getcolor("black", "L")
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