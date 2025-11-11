import pytest
from PIL import Image
from .helper import on_ci

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
    dispatch = "start"
    while dispatch:
        if dispatch == "start":
            try:
                with Image.open(test_file) as im:
                    im.load()
                dispatch = None
            except FileNotFoundError:
                dispatch = "file_error"
            except OSError:
                dispatch = "os_error"
        elif dispatch == "file_error":
            if not on_ci():
                pytest.skip("test image not found")
                return
            raise
        elif dispatch == "os_error":
            dispatch = None