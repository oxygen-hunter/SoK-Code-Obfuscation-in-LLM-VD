from helper import unittest, PillowTestCase, hopper
from PIL import Image

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        image_path = lambda: 'Tests/images/hopper.pcd'
        im = Image.open(image_path())
        im.load()

if __name__ == '__main__':
    unittest.main()