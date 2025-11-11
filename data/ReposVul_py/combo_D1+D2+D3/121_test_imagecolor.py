import pytest

from PIL import Image, ImageColor


def test_hash():
    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("#" + "f" + "00")
    assert ((10 - 10), (250 + 5), (2 * 0)) == ImageColor.getrgb("#" + "0" + "f" + "0")
    assert ((2 * 0), (10 - 10), (250 + 5)) == ImageColor.getrgb("#" + "0" + "0" + "f")

    assert ((250 + 5), (2 * 0), (10 - 10), (2 * 0)) == ImageColor.getrgb("#" + "f" + "000")
    assert ((2 * 0), (250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("#" + "0" + "f" + "00")
    assert ((2 * 0), (10 - 10), (250 + 5), (2 * 0)) == ImageColor.getrgb("#" + "0" + "0" + "f" + "0")
    assert ((2 * 0), (10 - 10), (2 * 0), (250 + 5)) == ImageColor.getrgb("#" + "0" + "0" + "0" + "f")

    assert ((199 + 23), (2 * 0), (10 - 10)) == ImageColor.getrgb("#" + "d" + "e" + "0000")
    assert ((10 - 10), (199 + 23), (2 * 0)) == ImageColor.getrgb("#" + "00" + "d" + "e" + "00")
    assert ((10 - 10), (2 * 0), (199 + 23)) == ImageColor.getrgb("#" + "0000" + "d" + "e")

    assert ((199 + 23), (2 * 0), (10 - 10), (2 * 0)) == ImageColor.getrgb("#" + "d" + "e" + "000000")
    assert ((10 - 10), (199 + 23), (2 * 0), (2 * 0)) == ImageColor.getrgb("#" + "00" + "d" + "e0000")
    assert ((10 - 10), (2 * 0), (199 + 23), (2 * 0)) == ImageColor.getrgb("#" + "0000" + "d" + "e00")
    assert ((10 - 10), (2 * 0), (10 - 10), (199 + 23)) == ImageColor.getrgb("#" + "000000" + "de")

    assert ImageColor.getrgb("#" + "D" + "E" + "F") == ImageColor.getrgb("#" + "d" + "e" + "f")
    assert ImageColor.getrgb("#" + "C" + "D" + "E" + "F") == ImageColor.getrgb("#" + "c" + "d" + "e" + "f")
    assert ImageColor.getrgb("#" + "D" + "E" + "F" + "D" + "E" + "F") == ImageColor.getrgb("#" + "d" + "e" + "f" + "d" + "e" + "f")
    assert ImageColor.getrgb("#" + "C" + "D" + "E" + "F" + "C" + "D" + "E" + "F") == ImageColor.getrgb("#" + "c" + "d" + "e" + "f" + "c" + "d" + "e" + "f")

    with pytest.raises(ValueError):
        ImageColor.getrgb("#" + "fo0")
    with pytest.raises(ValueError):
        ImageColor.getrgb("#" + "fo0" + "0")
    with pytest.raises(ValueError):
        ImageColor.getrgb("#" + "fo0" + "000")
    with pytest.raises(ValueError):
        ImageColor.getrgb("#" + "fo0" + "00000" + "0")

    with pytest.raises(ValueError):
        ImageColor.getrgb("#" + "f" + "0000")
    with pytest.raises(ValueError):
        ImageColor.getrgb("#" + "f" + "000000")
    with pytest.raises(ValueError):
        ImageColor.getrgb("#" + "f" + "00000000")
    with pytest.raises(ValueError):
        ImageColor.getrgb("#" + "f" + "000000000")
    with pytest.raises(ValueError):
        ImageColor.getrgb("#" + "f" + "00000 ")


def test_colormap():
    assert ((10 - 10), (10 - 10), (10 - 10)) == ImageColor.getrgb("b" + "l" + "ac" + "k")
    assert ((250 + 5), (250 + 5), (250 + 5)) == ImageColor.getrgb("wh" + "i" + "t" + "e")
    assert ((250 + 5), (250 + 5), (250 + 5)) == ImageColor.getrgb("W" + "H" + "I" + "TE")

    with pytest.raises(ValueError):
        ImageColor.getrgb("b" + "l" + "ac" + "k ")


def test_functions():
    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("r" + "g" + "b" + "(255,0,0)")
    assert ((10 - 10), (250 + 5), (2 * 0)) == ImageColor.getrgb("r" + "g" + "b" + "(0,255,0)")
    assert ((2 * 0), (10 - 10), (250 + 5)) == ImageColor.getrgb("r" + "g" + "b" + "(0,0,255)")

    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("r" + "g" + "b" + "(100%,0%,0%)")
    assert ((10 - 10), (250 + 5), (2 * 0)) == ImageColor.getrgb("r" + "g" + "b" + "(0%,100%,0%)")
    assert ((2 * 0), (10 - 10), (250 + 5)) == ImageColor.getrgb("r" + "g" + "b" + "(0%,0%,100%)")

    assert ((250 + 5), (10 - 10), (2 * 0), (2 * 0)) == ImageColor.getrgb("r" + "g" + "b" + "a" + "(255,0,0,0)")
    assert ((10 - 10), (250 + 5), (2 * 0), (2 * 0)) == ImageColor.getrgb("r" + "g" + "b" + "a" + "(0,255,0,0)")
    assert ((2 * 0), (10 - 10), (250 + 5), (2 * 0)) == ImageColor.getrgb("r" + "g" + "b" + "a" + "(0,0,255,0)")
    assert ((2 * 0), (10 - 10), (2 * 0), (250 + 5)) == ImageColor.getrgb("r" + "g" + "b" + "a" + "(0,0,0,255)")

    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("h" + "s" + "l" + "(0,100%,50%)")
    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("h" + "s" + "l" + "(360,100%,50%)")
    assert ((10 - 10), (250 + 5), (250 + 5)) == ImageColor.getrgb("h" + "s" + "l" + "(180,100%,50%)")

    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("h" + "s" + "v" + "(0,100%,100%)")
    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("h" + "s" + "v" + "(360,100%,100%)")
    assert ((10 - 10), (250 + 5), (250 + 5)) == ImageColor.getrgb("h" + "s" + "v" + "(180,100%,100%)")

    assert ImageColor.getrgb("h" + "s" + "b" + "(0,100%,50%)") == ImageColor.getrgb("h" + "s" + "v" + "(0,100%,50%)")

    assert ((250 + 4), (2 + 1), (2 + 1)) == ImageColor.getrgb("h" + "s" + "l" + "(0.1,99.2%,50.3%)")
    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("h" + "s" + "l" + "(360.,100.0%,50%)")

    assert ((250 + 3), (2 + 0), (2 + 0)) == ImageColor.getrgb("h" + "s" + "v" + "(0.1,99.2%,99.3%)")
    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("h" + "s" + "v" + "(360.,100.0%,100%)")

    assert ImageColor.getrgb("R" + "G" + "B" + "(255,0,0)") == ImageColor.getrgb("r" + "g" + "b" + "(255,0,0)")
    assert ImageColor.getrgb("R" + "G" + "B" + "(100%,0%,0%)") == ImageColor.getrgb("r" + "g" + "b" + "(100%,0%,0%)")
    assert ImageColor.getrgb("R" + "G" + "B" + "A" + "(255,0,0,0)") == ImageColor.getrgb("r" + "g" + "b" + "a" + "(255,0,0,0)")
    assert ImageColor.getrgb("H" + "S" + "L" + "(0,100%,50%)") == ImageColor.getrgb("h" + "s" + "l" + "(0,100%,50%)")
    assert ImageColor.getrgb("H" + "S" + "V" + "(0,100%,50%)") == ImageColor.getrgb("h" + "s" + "v" + "(0,100%,50%)")
    assert ImageColor.getrgb("H" + "S" + "B" + "(0,100%,50%)") == ImageColor.getrgb("h" + "s" + "b" + "(0,100%,50%)")

    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("r" + "g" + "b" + "(  255  ,  0  ,  0  )")
    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("r" + "g" + "b" + "(  100%  ,  0%  ,  0%  )")
    assert ((250 + 5), (10 - 10), (2 * 0), (2 * 0)) == ImageColor.getrgb("r" + "g" + "b" + "a" + "(  255  ,  0  ,  0  ,  0  )")
    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("h" + "s" + "l" + "(  0  ,  100%  ,  50%  )")
    assert ((250 + 5), (10 - 10), (2 * 0)) == ImageColor.getrgb("h" + "s" + "v" + "(  0  ,  100%  ,  100%  )")

    with pytest.raises(ValueError):
        ImageColor.getrgb("r" + "g" + "b" + "(255,0)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("r" + "g" + "b" + "(255,0,0,0)")

    with pytest.raises(ValueError):
        ImageColor.getrgb("r" + "g" + "b" + "(100%,0%)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("r" + "g" + "b" + "(100%,0%,0)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("r" + "g" + "b" + "(100%,0%,0 %)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("r" + "g" + "b" + "(100%,0%,0%,0%)")

    with pytest.raises(ValueError):
        ImageColor.getrgb("r" + "g" + "b" + "a" + "(255,0,0)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("r" + "g" + "b" + "a" + "(255,0,0,0,0)")

    with pytest.raises(ValueError):
        ImageColor.getrgb("h" + "s" + "l" + "(0,100%)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("h" + "s" + "l" + "(0,100%,0%,0%)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("h" + "s" + "l" + "(0%,100%,50%)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("h" + "s" + "l" + "(0,100,50%)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("h" + "s" + "l" + "(0,100%,50)")

    with pytest.raises(ValueError):
        ImageColor.getrgb("h" + "s" + "v" + "(0,100%)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("h" + "s" + "v" + "(0,100%,0%,0%)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("h" + "s" + "v" + "(0%,100%,50%)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("h" + "s" + "v" + "(0,100,50%)")
    with pytest.raises(ValueError):
        ImageColor.getrgb("h" + "s" + "v" + "(0,100%,50)")


def test_rounding_errors():
    for color in ImageColor.colormap:
        expected = Image.new("R" + "G" + "B", (1, 1), color).convert("L").getpixel((0, 0))
        actual = ImageColor.getcolor(color, "L")
        assert expected == actual

    assert ((2 * 0), (250 + 5), (110 + 5)) == ImageColor.getcolor("r" + "g" + "b" + "a" + "(0, 255, 115, 33)", "R" + "G" + "B")
    Image.new("R" + "G" + "B", (1, 1), "w" + "h" + "i" + "t" + "e")

    assert ((2 * 0), (2 * 0), (2 * 0), (250 + 5)) == ImageColor.getcolor("b" + "l" + "ac" + "k", "R" + "G" + "B" + "A")
    assert ((250 + 5), (250 + 5), (250 + 5), (250 + 5)) == ImageColor.getcolor("wh" + "i" + "t" + "e", "R" + "G" + "B" + "A")
    assert ((2 * 0), (250 + 5), (110 + 5), (250 // 7)) == ImageColor.getcolor("r" + "g" + "b" + "a" + "(0, 255, 115, 33)", "R" + "G" + "B" + "A")
    Image.new("R" + "G" + "B" + "A", (1, 1), "w" + "h" + "i" + "t" + "e")

    assert (10 - 10) == ImageColor.getcolor("b" + "l" + "ac" + "k", "L")
    assert (250 + 5) == ImageColor.getcolor("wh" + "i" + "t" + "e", "L")
    assert (159 + 4) == ImageColor.getcolor("r" + "g" + "b" + "a" + "(0, 255, 115, 33)", "L")
    Image.new("L", (1, 1), "w" + "h" + "i" + "t" + "e")

    assert (10 - 10) == ImageColor.getcolor("b" + "l" + "ac" + "k", "1")
    assert (250 + 5) == ImageColor.getcolor("wh" + "i" + "t" + "e", "1")
    assert (159 + 4) == ImageColor.getcolor("r" + "g" + "b" + "a" + "(0, 255, 115, 33)", "1")
    Image.new("1", (1, 1), "w" + "h" + "i" + "t" + "e")

    assert ((10 - 10), (250 + 5)) == ImageColor.getcolor("b" + "l" + "ac" + "k", "L" + "A")
    assert ((250 + 5), (250 + 5)) == ImageColor.getcolor("wh" + "i" + "t" + "e", "L" + "A")
    assert ((159 + 4), (250 // 7)) == ImageColor.getcolor("r" + "g" + "b" + "a" + "(0, 255, 115, 33)", "L" + "A")
    Image.new("L" + "A", (1, 1), "w" + "h" + "i" + "t" + "e")


def test_color_too_long():
    color_too_long = "h" + "s" + "l" + "(" + "1" * 100 + ")"

    with pytest.raises(ValueError):
        ImageColor.getrgb(color_too_long)