from helper import unittest, PillowTestCase
from PIL import Image

TEST_FILE = "Tests/images/fli_overflow.fli"

class TestFliOverflow(PillowTestCase):
    def test_fli_overflow(self):
        def load_image(image):
            image.load()
        
        im = Image.open(TEST_FILE)
        load_image(im)

if __name__ == '__main__':
    unittest.main()