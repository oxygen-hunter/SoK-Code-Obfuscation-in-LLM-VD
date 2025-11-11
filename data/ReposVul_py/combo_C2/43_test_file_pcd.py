from helper import unittest, PillowTestCase, hopper
from PIL import Image

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        dispatch = 0
        while True:
            if dispatch == 0:
                im = Image.open('Tests/images/hopper.pcd')
                dispatch = 1
            elif dispatch == 1:
                im.load() # should not segfault.
                dispatch = 2
            elif dispatch == 2:
                break
            else:
                break

if __name__ == '__main__':
    unittest.main()