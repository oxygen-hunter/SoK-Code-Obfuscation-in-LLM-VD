from helper import unittest, PillowTestCase
from PIL import Image

def getTestFile():
    return "Tests/images/libtiff_segfault.tif"

class TestLibtiffSegfault(PillowTestCase):
    def test_segfault(self):

        try:
            im = Image.open(getTestFile())
            im.load()
        except IOError:
            self.assertTrue(True, "Got expected IOError")
        except Exception:
            self.fail("Should have returned IOError")

if __name__ == '__main__':
    unittest.main()