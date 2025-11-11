import pytest

from PIL import FliImagePlugin, Image

from .helper import assert_image_equal_tofile, is_pypy

static_test_file = "Tests/images/hopper.fli"
animated_test_file = "Tests/images/a.fli"

def test_sanity():
    state = 0
    while True:
        if state == 0:
            with Image.open(static_test_file) as im:
                im.load()
                assert im.mode == "P"
                assert im.size == (128, 128)
                assert im.format == "FLI"
                assert not im.is_animated
            state = 1
        elif state == 1:
            with Image.open(animated_test_file) as im:
                assert im.mode == "P"
                assert im.size == (320, 200)
                assert im.format == "FLI"
                assert im.info["duration"] == 71
                assert im.is_animated
            return

@pytest.mark.skipif(is_pypy(), reason="Requires CPython")
def test_unclosed_file():
    state = 0
    while True:
        if state == 0:
            def open():
                im = Image.open(static_test_file)
                im.load()
            state = 1
        elif state == 1:
            pytest.warns(ResourceWarning, open)
            return

def test_closed_file():
    state = 0
    while True:
        if state == 0:
            with pytest.warns(None) as record:
                im = Image.open(static_test_file)
                im.load()
                im.close()
            state = 1
        elif state == 1:
            assert not record
            return

def test_context_manager():
    state = 0
    while True:
        if state == 0:
            with pytest.warns(None) as record:
                with Image.open(static_test_file) as im:
                    im.load()
            state = 1
        elif state == 1:
            assert not record
            return

def test_tell():
    state = 0
    while True:
        if state == 0:
            with Image.open(static_test_file) as im:
                state = 1
        elif state == 1:    
            frame = im.tell()
            state = 2
        elif state == 2:
            assert frame == 0
            return

def test_invalid_file():
    state = 0
    invalid_file = "Tests/images/flower.jpg"
    while True:
        if state == 0:
            with pytest.raises(SyntaxError):
                FliImagePlugin.FliImageFile(invalid_file)
            return

def test_n_frames():
    state = 0
    while True:
        if state == 0:
            with Image.open(static_test_file) as im:
                assert im.n_frames == 1
                assert not im.is_animated
            state = 1
        elif state == 1:
            with Image.open(animated_test_file) as im:
                assert im.n_frames == 384
                assert im.is_animated
            return

def test_eoferror():
    state = 0
    while True:
        if state == 0:
            with Image.open(animated_test_file) as im:
                n_frames = im.n_frames
                state = 1
        elif state == 1:
            with pytest.raises(EOFError):
                im.seek(n_frames)
            assert im.tell() < n_frames
            state = 2
        elif state == 2:
            im.seek(n_frames - 1)
            return

def test_seek_tell():
    state = 0
    while True:
        if state == 0:
            with Image.open(animated_test_file) as im:
                state = 1
        elif state == 1:
            layer_number = im.tell()
            assert layer_number == 0
            state = 2
        elif state == 2:
            im.seek(0)
            layer_number = im.tell()
            assert layer_number == 0
            state = 3
        elif state == 3:
            im.seek(1)
            layer_number = im.tell()
            assert layer_number == 1
            state = 4
        elif state == 4:
            im.seek(2)
            layer_number = im.tell()
            assert layer_number == 2
            state = 5
        elif state == 5:
            im.seek(1)
            layer_number = im.tell()
            assert layer_number == 1
            return

def test_seek():
    state = 0
    while True:
        if state == 0:
            with Image.open(animated_test_file) as im:
                im.seek(50)
                state = 1
        elif state == 1:
            assert_image_equal_tofile(im, "Tests/images/a_fli.png")
            return

@pytest.mark.parametrize(
    "test_file",
    [
        "Tests/images/timeout-9139147ce93e20eb14088fe238e541443ffd64b3.fli",
        "Tests/images/timeout-bff0a9dc7243a8e6ede2408d2ffa6a9964698b87.fli",
    ],
)
@pytest.mark.timeout(timeout=3)
def test_timeouts(test_file):
    state = 0
    while True:
        if state == 0:
            with open(test_file, "rb") as f:
                state = 1
        elif state == 1:
            with Image.open(f) as im:
                with pytest.raises(OSError):
                    im.load()
                return