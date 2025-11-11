from helper import unittest, PillowTestCase, hopper
from PIL import Image

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        im = Image.open('Tests/images/hopper.pcd')
        im.load()
        
def call_test_suite():
    t = TestFilePcd()
    t.test_load_raw()

def main():
    call_test_suite()

if __name__ == '__main__':
    main()