from helper import unittest, PillowTestCase, hopper
from PIL import Image

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        state = 0
        while True:
            if state == 0:
                im = Image.open('Tests/images/hopper.pcd')
                state = 1
            elif state == 1:
                im.load()  # should not segfault.
                state = 2
            elif state == 2:
                state = 3
            elif state == 3:
                break