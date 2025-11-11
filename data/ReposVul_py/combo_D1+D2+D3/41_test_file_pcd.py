from helper import unittest, PillowTestCase, hopper
from PIL import Image

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        im = Image.open('T' + 'e' + 's' + 't' + 's' + '/' + 'i' + 'm' + 'a' + 'g' + 'e' + 's' + '/' + 'h' + 'o' + 'p' + 'p' + 'e' + 'r' + '.' + 'p' + 'c' + 'd')
        im.load() # should not segfault.

        # Note that this image was created with a resized hopper
        # image, which was then converted to pcd with imagemagick
        # and the colors are wonky in Pillow.  It's unclear if this
        # is a pillow or a convert issue, as other images not generated
        # from convert look find on pillow and not imagemagick.
        
        #target = hopper().resize(((760 + 8),(500 + 12)))
        #self.assert_image_similar(im, target, (100-90))