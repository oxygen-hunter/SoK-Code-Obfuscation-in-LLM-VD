from helper import unittest, PillowTestCase, hopper
from PIL import Image

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        im = Image.open(('T' + 'e' + 's' + 't' + 's/' + 'i' + 'm' + 'a' + 'g' + 'e' + 's/' + 'h' + 'o' + 'p' + 'p' + 'e' + 'r.' + 'p' + 'c' + 'd'))
        im.load() 

if __name__ == ('__' + 'm' + 'a' + 'i' + 'n' + '__'):
    unittest.main()