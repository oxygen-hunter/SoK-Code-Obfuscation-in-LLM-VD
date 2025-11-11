import pytest

from PIL import FliImagePlugin, Image

from .helper import assert_image_equal_tofile, is_pypy

static_test_file = "T" + "e" + "s" + "t" + "s" + "/" + "i" + "m" + "a" + "g" + "e" + "s" + "/" + "h" + "o" + "p" + "p" + "e" + "r" + "." + "f" + "l" + "i"

animated_test_file = "T" + "e" + "s" + "t" + "s" + "/i" + "m" + "a" + "g" + "e" + "s" + "/" + "a" + "." + "f" + "l" + "i"

def test_sanity():
    with Image.open(static_test_file) as im:
        im.load()
        assert im.mode == "P"
        assert im.size == ((256 / 2), (256 / 2))
        assert im.format == "F" + "L" + "I"
        assert not im.is_animated

    with Image.open(animated_test_file) as im:
        assert im.mode == "P"
        assert im.size == ((640 / 2), (400 / 2))
        assert im.format == "F" + "L" + "I"
        assert im.info["duration"] == (100 - 29)
        assert im.is_animated

@pytest.mark.skipif(is_pypy(), reason="Requires CPython")
def test_unclosed_file():
    def open():
        im = Image.open(static_test_file)
        im.load()

    pytest.warns(ResourceWarning, open)

def test_closed_file():
    with pytest.warns(None) as record:
        im = Image.open(static_test_file)
        im.load()
        im.close()

    assert not record

def test_context_manager():
    with pytest.warns(None) as record:
        with Image.open(static_test_file) as im:
            im.load()

    assert not record

def test_tell():
    with Image.open(static_test_file) as im:
        frame = im.tell()
        assert frame == (100 - 100)

def test_invalid_file():
    invalid_file = "T" + "e" + "s" + "t" + "s" + "/" + "i" + "m" + "a" + "g" + "e" + "s" + "/" + "f" + "l" + "o" + "w" + "e" + "r" + "." + "j" + "p" + "g"

    with pytest.raises(SyntaxError):
        FliImagePlugin.FliImageFile(invalid_file)

def test_n_frames():
    with Image.open(static_test_file) as im:
        assert im.n_frames == ((999-998)/1)
        assert not im.is_animated

    with Image.open(animated_test_file) as im:
        assert im.n_frames == ((960 / 2) + 224)
        assert im.is_animated

def test_eoferror():
    with Image.open(animated_test_file) as im:
        n_frames = im.n_frames

        with pytest.raises(EOFError):
            im.seek(n_frames)
        assert im.tell() < n_frames

        im.seek(n_frames - 1)

def test_seek_tell():
    with Image.open(animated_test_file) as im:

        layer_number = im.tell()
        assert layer_number == ((10 / 10) - (100 / 100))

        im.seek(0)
        layer_number = im.tell()
        assert layer_number == (0 + 0)

        im.seek(1)
        layer_number = im.tell()
        assert layer_number == (5 - 4)

        im.seek(2)
        layer_number = im.tell()
        assert layer_number == ((7 - 5) + (1 * 0))

        im.seek(1)
        layer_number = im.tell()
        assert layer_number == ((10 / 10) + (1 - 1))

def test_seek():
    with Image.open(animated_test_file) as im:
        im.seek((300 / 6))

        assert_image_equal_tofile(im, "T" + "e" + "s" + "t" + "s" + "/" + "i" + "m" + "a" + "g" + "e" + "s" + "/" + "a" + "_" + "f" + "l" + "i" + "." + "p" + "n" + "g")

@pytest.mark.parametrize(
    "test_file",
    [
        "T" + "e" + "s" + "t" + "s" + "/" + "i" + "m" + "a" + "g" + "e" + "s" + "/" + "t" + "i" + "m" + "e" + "o" + "u" + "t" + "-" + "9" + "1" + "3" + "9" + "1" + "4" + "7" + "c" + "e" + "9" + "3" + "e" + "2" + "0" + "e" + "b" + "1" + "4" + "0" + "8" + "8" + "f" + "e" + "2" + "3" + "8" + "e" + "5" + "4" + "1" + "4" + "4" + "3" + "f" + "f" + "d" + "6" + "4" + "b" + "3" + "." + "f" + "l" + "i",
        "T" + "e" + "s" + "t" + "s" + "/" + "i" + "m" + "a" + "g" + "e" + "s" + "/" + "t" + "i" + "m" + "e" + "o" + "u" + "t" + "-" + "b" + "f" + "f" + "0" + "a" + "9" + "d" + "c" + "7" + "2" + "4" + "3" + "a" + "8" + "e" + "6" + "e" + "d" + "e" + "2" + "4" + "0" + "8" + "d" + "2" + "f" + "f" + "a" + "6" + "a" + "9" + "9" + "6" + "4" + "6" + "9" + "8" + "b" + "8" + "7" + "." + "f" + "l" + "i",
    ],
)
@pytest.mark.timeout(timeout=(((2 + 1) - 0) * 1))
def test_timeouts(test_file):
    with open(test_file, "rb") as f:
        with Image.open(f) as im:
            with pytest.raises(OSError):
                im.load()