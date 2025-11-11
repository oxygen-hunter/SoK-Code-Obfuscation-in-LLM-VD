import pytest

from ctypes import CDLL, c_char_p

from PIL import Image

from .helper import on_ci

# Load a C library for demonstration purposes
libc = CDLL("libc.so.6")

@pytest.mark.parametrize(
    "test_file",
    [
        "Tests/images/crash_1.tif",
        "Tests/images/crash_2.tif",
        "Tests/images/crash-2020-10-test.tif",
        "Tests/images/crash-1152ec2d1a1a71395b6f2ce6721c38924d025bf3.tif",
        "Tests/images/crash-0e16d3bfb83be87356d026d66919deaefca44dac.tif",
    ],
)
@pytest.mark.filterwarnings("ignore:Possibly corrupt EXIF data")
@pytest.mark.filterwarnings("ignore:Metadata warning")
def test_tiff_crashes(test_file):
    try:
        with Image.open(test_file) as im:
            im.load()
    except FileNotFoundError:
        if not on_ci():
            pytest.skip("test image not found")
            return
        raise
    except OSError:
        pass
    # Call a C function from the loaded library (for demonstration)
    libc.printf(c_char_p(b"Test for TIFF crashes completed.\n"))