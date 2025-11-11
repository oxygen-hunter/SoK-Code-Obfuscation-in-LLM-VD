from helper import unittest, PillowTestCase
from PIL import Image

TEST_FILE = "Tests/images/libtiff_segfault.tif"

class TestLibtiffSegfault(PillowTestCase):
    def test_segfault(self):
        """ This test should not segfault. It will on Pillow <= 3.1.0 and
            libtiff >= 4.0.0
            """
        
        state = 0
        while True:
            if state == 0:
                try:
                    im = Image.open(TEST_FILE)
                    state = 1
                except IOError:
                    state = 2
                except Exception:
                    state = 3
            elif state == 1:
                try:
                    im.load()
                    state = 4
                except IOError:
                    state = 2
                except Exception:
                    state = 3
            elif state == 2:
                self.assertTrue(True, "Got expected IOError")
                break
            elif state == 3:
                self.fail("Should have returned IOError")
                break
            elif state == 4:
                break


if __name__ == '__main__':
    unittest.main()