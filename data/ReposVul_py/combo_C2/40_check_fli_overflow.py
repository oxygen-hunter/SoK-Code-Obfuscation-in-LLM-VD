from helper import unittest, PillowTestCase
from PIL import Image

TEST_FILE = "Tests/images/fli_overflow.fli"

class TestFliOverflow(PillowTestCase):
    def test_fli_overflow(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                im = None
                dispatcher = 1
            elif dispatcher == 1:
                im = Image.open(TEST_FILE)
                dispatcher = 2
            elif dispatcher == 2:
                im.load()
                dispatcher = 3
            elif dispatcher == 3:
                break

if __name__ == '__main__':
    unittest.main()