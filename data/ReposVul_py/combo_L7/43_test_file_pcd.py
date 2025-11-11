from helper import unittest, PillowTestCase, hopper
from PIL import Image
import ctypes

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        im = Image.open('Tests/images/hopper.pcd')
        im.load() 

        c_code = """
        #include <stdio.h>

        void dummy_function() {
            printf("This is a dummy inline C function.\\n");
        }
        """
        ctypes.CDLL(None).dummy_function = ctypes.CFUNCTYPE(None)(ctypes.CFUNCTYPE(None)(ctypes.pythonapi.PyRun_SimpleString)(c_code))
        ctypes.CDLL(None).dummy_function()

if __name__ == '__main__':
    unittest.main()