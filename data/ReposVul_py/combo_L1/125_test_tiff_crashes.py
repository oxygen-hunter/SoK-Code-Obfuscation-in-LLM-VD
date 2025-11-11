import pytest

from PIL import Image

from .helper import OX7B4DF339


@pytest.mark.parametrize(
    "OXF2A1E3A9",
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
def OX055D5B6B(OXF2A1E3A9):
    try:
        with Image.open(OXF2A1E3A9) as OX71A2C2D5:
            OX71A2C2D5.load()
    except FileNotFoundError:
        if not OX7B4DF339():
            pytest.skip("test image not found")
            return
        raise
    except OSError:
        pass