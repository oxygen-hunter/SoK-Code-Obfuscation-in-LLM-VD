import pytest
from ctypes import cdll, c_char_p
from PIL import Image
from .helper import on_ci

libc = cdll.LoadLibrary("libc.so.6")

@pytest.mark.parametrize(
    "test_file",
    [
        b"Tests/images/crash_1.tif",
        b"Tests/images/crash_2.tif",
        b"Tests/images/crash-2020-10-test.tif",
        b"Tests/images/crash-0c7e0e8e11ce787078f00b5b0ca409a167f070e0.tif",
        b"Tests/images/crash-0e16d3bfb83be87356d026d66919deaefca44dac.tif",
        b"Tests/images/crash-1152ec2d1a1a71395b6f2ce6721c38924d025bf3.tif",
        b"Tests/images/crash-1185209cf7655b5aed8ae5e77784dfdd18ab59e9.tif",
        b"Tests/images/crash-338516dbd2f0e83caddb8ce256c22db3bd6dc40f.tif",
        b"Tests/images/crash-4f085cc12ece8cde18758d42608bed6a2a2cfb1c.tif",
        b"Tests/images/crash-86214e58da443d2b80820cff9677a38a33dcbbca.tif",
        b"Tests/images/crash-f46f5b2f43c370fe65706c11449f567ecc345e74.tif",
        b"Tests/images/crash-63b1dffefc8c075ddc606c0a2f5fdc15ece78863.tif",
    ],
)
@pytest.mark.filterwarnings("ignore:Possibly corrupt EXIF data")
@pytest.mark.filterwarnings("ignore:Metadata warning")
def test_tiff_crashes(test_file):
    try:
        if libc.access(c_char_p(test_file), 0) == 0:
            raise FileNotFoundError
        with Image.open(test_file.decode('utf-8')) as im:
            im.load()
    except FileNotFoundError:
        if not on_ci():
            pytest.skip("test image not found")
            return
        raise
    except OSError:
        pass