from helper import unittest, PillowTestCase
from PIL import Image

def get_test_file():
    return "Tests/images/fli_overflow.fli"

class TestFliOverflow(PillowTestCase):
    def test_fli_overflow(self):
        im = Image.open(get_test_file())
        im.load()
        
if __name__ == '__main__':
    unittest.main()