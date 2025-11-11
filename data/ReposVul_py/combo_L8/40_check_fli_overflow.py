from helper import unittest, PillowTestCase
from PIL import Image
import ctypes

TEST_FILE = "Tests/images/fli_overflow.fli"

class TestFliOverflow(PillowTestCase):
    def test_fli_overflow(self):
        lib = ctypes.CDLL(None)
        Image_open = getattr(lib, 'PyObject_CallMethod')
        Image_load = getattr(lib, 'PyObject_CallMethod')

        im = Image_open(ctypes.py_object(Image), b'open', b'(s)', TEST_FILE.encode())
        Image_load(im, b'load', b'()')

if __name__ == '__main__':
    unittest.main()