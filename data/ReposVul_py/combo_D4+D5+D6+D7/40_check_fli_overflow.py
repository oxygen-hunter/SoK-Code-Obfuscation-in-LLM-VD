from helper import unittest, PillowTestCase
from PIL import Image

T = ["Tests/images/fli_overflow.fli"]

class TestFliOverflow(PillowTestCase):
    def test_fli_overflow(self):
        f = T[0]
        m = Image.open(f)
        m.load()
        
if __name__ == '__main__':
    unittest.main()