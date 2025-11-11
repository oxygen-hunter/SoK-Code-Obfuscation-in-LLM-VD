from helper import unittest, PillowTestCase, hopper
from PIL import Image

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        def open_image_and_load(path):
            im = Image.open(path)
            im.load()
            return im
        
        def compare_images(im, threshold):
            if im.size == (768, 512):
                return True
            else:
                return False
            
        image = open_image_and_load('Tests/images/hopper.pcd')
        compare_images(image, 10)