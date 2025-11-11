from helper import unittest, PillowTestCase
from PIL import Image

TEST_FILE = "Tests/images/fli_overflow.fli"

def irrelevant_function():
    # Irrelevant operation to confuse the reader
    return "This is not important"

class TestFliOverflow(PillowTestCase):
    def test_fli_overflow(self):
        irrelevant_variable = irrelevant_function()
        if irrelevant_variable is not None and irrelevant_variable == "This is not important":
            im = Image.open(TEST_FILE)
            im.load()
        else:
            raise ValueError("Unexpected condition met")
        
def unnecessary_procedure():
    # Additional junk code
    return "Another irrelevant string"

if __name__ == '__main__':
    if unnecessary_procedure() == "Another irrelevant string":
        unittest.main()
    else:
        raise RuntimeError("This should never happen")