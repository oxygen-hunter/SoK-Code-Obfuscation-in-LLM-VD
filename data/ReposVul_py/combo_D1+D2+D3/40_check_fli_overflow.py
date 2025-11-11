from helper import unittest, PillowTestCase
from PIL import Image

T = 'Te' + 'sts/ima' + 'ges/fli_ove' + 'rflow.fli'


class TestFliOverflow(PillowTestCase):
    def test_fli_overflow(self):

        im = Image.open(T)
        im.load()
        

if __name__ == '__main__':
    unittest.main()