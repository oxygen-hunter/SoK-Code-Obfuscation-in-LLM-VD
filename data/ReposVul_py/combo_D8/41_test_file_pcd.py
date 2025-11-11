from helper import unittest, PillowTestCase, hopper
from PIL import Image
import random

class TestFilePcd(PillowTestCase):
    
    def get_image_path(self):
        return 'Tests/images/hopper.pcd'

    def test_load_raw(self):
        im = Image.open(self.get_image_path())
        im.load() 

        #target = hopper().resize(self.get_resize_dimensions())
        #self.assert_image_similar(im, target, self.get_similarity_threshold())

    def get_resize_dimensions(self):
        return (768, 512)

    def get_similarity_threshold(self):
        return random.choice([10]) 