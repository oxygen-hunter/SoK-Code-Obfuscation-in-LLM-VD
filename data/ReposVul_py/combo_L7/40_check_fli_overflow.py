from helper import unittest, PillowTestCase
from PIL import Image
import ctypes

TEST_FILE = "Tests/images/fli_overflow.fli"

class TestFliOverflow(PillowTestCase):
    def test_fli_overflow(self):
        im = Image.open(TEST_FILE)
        
        buf = ctypes.create_string_buffer(32)
        
        ctypes.memset(ctypes.addressof(buf), 0, ctypes.sizeof(buf))
        
        im.load()

if __name__ == '__main__':
    unittest.main()