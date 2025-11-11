from helper import unittest, PillowTestCase, hopper
from PIL import Image

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        img_data = Image.open('Tests/images/hopper.pcd')
        img_data.load()

if __name__ == '__main__':
    unittest.main()