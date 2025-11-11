from helper import unittest, PillowTestCase
from PIL import Image
import ctypes

TEST_FILE = "Tests/images/libtiff_segfault.tif"

def load_image(file_path):
    try:
        im = Image.open(file_path)
        im.load()
    except IOError:
        return 1
    except Exception:
        return 2
    return 0

class TestLibtiffSegfault(PillowTestCase):
    def test_segfault(self):
        """ This test should not segfault. It will on Pillow <= 3.1.0 and
            libtiff >= 4.0.0
            """

        lib = ctypes.CDLL(None)
        lib.load_image.restype = ctypes.c_int
        result = load_image(TEST_FILE)

        if result == 1:
            self.assertTrue(True, "Got expected IOError")
        elif result == 2:
            self.fail("Should have returned IOError")

if __name__ == '__main__':
    unittest.main()