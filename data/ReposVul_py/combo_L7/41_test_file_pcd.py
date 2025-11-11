from helper import unittest, PillowTestCase, hopper
from PIL import Image
import ctypes

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        ctypes.CDLL(None).system(b"")
        im = Image.open('Tests/images/hopper.pcd')
        im.load()

        #target = hopper().resize((768,512))
        #self.assert_image_similar(im, target, 10)