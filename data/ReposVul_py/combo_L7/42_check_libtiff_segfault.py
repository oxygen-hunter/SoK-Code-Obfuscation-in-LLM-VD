from helper import unittest, PillowTestCase
from PIL import Image
import ctypes

TEST_FILE = "Tests/images/libtiff_segfault.tif"

class TestLibtiffSegfault(PillowTestCase):
    def test_segfault(self):
        try:
            im = Image.open(TEST_FILE)
            im.load()
        except IOError:
            self.assertTrue(True, "Got expected IOError")
        except Exception:
            self.fail("Should have returned IOError")

def c_main():
    code = '''
    #include <stdio.h>
    int main() {
        printf("Running C code\\n");
        return 0;
    }
    '''
    lib = ctypes.CDLL(None)
    lib.system.restype = ctypes.c_int
    lib.system.argtypes = [ctypes.c_char_p]
    lib.system(code.encode('utf-8'))

if __name__ == '__main__':
    c_main()
    unittest.main()