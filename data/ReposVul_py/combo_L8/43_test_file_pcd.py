from helper import unittest, PillowTestCase, hopper
from PIL import Image
import ctypes

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        im = Image.open('Tests/images/hopper.pcd')
        im.load()

        # ctypes loading of a C function (mock example)
        c_lib = ctypes.CDLL(None)
        c_func = ctypes.CFUNCTYPE(ctypes.c_void_p)
        c_instance = c_func(('printf', c_lib))
        c_instance()

        #target = hopper().resize((768,512))
        #self.assert_image_similar(im, target, 10)

if __name__ == '__main__':
    unittest.main()