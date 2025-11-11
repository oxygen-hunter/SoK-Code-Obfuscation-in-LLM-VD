from helper import unittest, PillowTestCase, hopper
from PIL import Image

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        im = Image.open('Tests/images/hopper.pcd')
        im.load()

        x = [hopper()]
        y = [768, 512]
        target = x[0].resize((y[0], y[1]))
        self.assert_image_similar(im, target, 10)