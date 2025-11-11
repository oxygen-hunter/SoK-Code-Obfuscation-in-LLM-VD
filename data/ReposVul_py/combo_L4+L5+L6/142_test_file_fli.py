import pytest

from PIL import FliImagePlugin, Image

from .helper import assert_image_equal_tofile, is_pypy

static_test_file = "Tests/images/hopper.fli"
animated_test_file = "Tests/images/a.fli"

def test_sanity():
    def check_image(file, mode, size, format, duration=None, animated=False):
        with Image.open(file) as im:
            im.load()
            assert im.mode == mode
            assert im.size == size
            assert im.format == format
            if duration is not None:
                assert im.info["duration"] == duration
            assert im.is_animated == animated
    
    check_image(static_test_file, "P", (128, 128), "FLI")
    check_image(animated_test_file, "P", (320, 200), "FLI", 71, True)

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
    def get_frame(file):
        with Image.open(file) as im:
            return im.tell()

    assert get_frame(static_test_file) == 0

def test_invalid_file():
    invalid_file = "Tests/images/flower.jpg"

    with pytest.raises(SyntaxError):
        FliImagePlugin.FliImageFile(invalid_file)

def test_n_frames():
    def check_frames(file, n_frames, animated):
        with Image.open(file) as im:
            assert im.n_frames == n_frames
            assert im.is_animated == animated

    check_frames(static_test_file, 1, False)
    check_frames(animated_test_file, 384, True)

def test_eoferror():
    def check_seek(file, n_frames):
        with Image.open(file) as im:
            with pytest.raises(EOFError):
                im.seek(n_frames)
            assert im.tell() < n_frames
            im.seek(n_frames - 1)
    
    with Image.open(animated_test_file) as im:
        n_frames = im.n_frames
        check_seek(animated_test_file, n_frames)

def test_seek_tell():
    def check_seek_tell(file, positions):
        with Image.open(file) as im:
            for pos in positions:
                im.seek(pos)
                assert im.tell() == pos

    check_seek_tell(animated_test_file, [0, 1, 2, 1])

def test_seek():
    def check_seek_image(file, frame, expected_file):
        with Image.open(file) as im:
            im.seek(frame)
            assert_image_equal_tofile(im, expected_file)

    check_seek_image(animated_test_file, 50, "Tests/images/a_fli.png")

@pytest.mark.parametrize(
    "test_file",
    [
        "Tests/images/timeout-9139147ce93e20eb14088fe238e541443ffd64b3.fli",
        "Tests/images/timeout-bff0a9dc7243a8e6ede2408d2ffa6a9964698b87.fli",
    ],
)
@pytest.mark.timeout(timeout=3)
def test_timeouts(test_file):
    def check_timeout(file):
        with open(file, "rb") as f:
            with Image.open(f) as im:
                with pytest.raises(OSError):
                    im.load()
    
    check_timeout(test_file)