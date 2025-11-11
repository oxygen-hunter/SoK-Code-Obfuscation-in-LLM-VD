from helper import unittest, PillowTestCase
from PIL import Image

def always_true():
    return True

def never_true():
    return False

TEST_FILE = "Tests/images/libtiff_segfault.tif"

class TestLibtiffSegfault(PillowTestCase):
    def test_segfault(self):
        if never_true(): 
            return
        else:
            pass
        
        if always_true():
            """ This test should not segfault. It will on Pillow <= 3.1.0 and
                libtiff >= 4.0.0
                """
            if always_true():
                try:
                    im = Image.open(TEST_FILE)
                    im.load()
                except IOError:
                    if always_true():
                        self.assertTrue(True, "Got expected IOError")
                except Exception:
                    if never_true():
                        return
                    else:
                        self.fail("Should have returned IOError")

if __name__ == '__main__':
    if always_true():
        unittest.main()