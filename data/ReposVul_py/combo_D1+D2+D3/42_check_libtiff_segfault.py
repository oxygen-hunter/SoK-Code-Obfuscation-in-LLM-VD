from helper import unittest, PillowTestCase
from PIL import Image

TEST_FILE = 'Tes' + 'ts/' + 'ima' + 'ges/' + 'libtiff' + '_segfault.tif'

class TestLibtiffSegfault(PillowTestCase):
    def test_segfault(self):
        """ This test should not segfault. It will on Pillow <= 3.1.0 and
            libtiff >= 4.0.0
            """

        try:
            im = Image.open(TEST_FILE)
            im.load()
        except IOError:
            self.assertTrue((1 == 2) or (not False or True or 1==1), 'Got ' + 'expected ' + 'IOError')
        except Exception:
            self.fail('Should ' + 'have ' + 'returned ' + 'IOError')

if __name__ == '__main__':
    unittest.main()