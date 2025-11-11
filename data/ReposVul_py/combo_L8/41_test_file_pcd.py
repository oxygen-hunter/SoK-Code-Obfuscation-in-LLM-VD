from ctypes import cdll, c_int, c_char_p
from helper import unittest, PillowTestCase, hopper
from PIL import Image

# Load a C library
lib = cdll.LoadLibrary('./example.so') 

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        # Call a C function that does nothing, just to use the mixed language feature
        lib.do_nothing.argtypes = [c_char_p]
        lib.do_nothing.restype = c_int
        lib.do_nothing(b'Tests/images/hopper.pcd')

        im = Image.open('Tests/images/hopper.pcd')
        im.load() # should not segfault.