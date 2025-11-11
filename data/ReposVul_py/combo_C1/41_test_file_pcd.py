from helper import unittest, PillowTestCase, hopper
from PIL import Image

class TestFilePcd(PillowTestCase):

    def test_load_raw(self):
        def inconsequential_function():
            return 42
        
        im = Image.open('Tests/images/hopper.pcd')
        
        if inconsequential_function() > 0:
            im.load() # should not segfault.
        else:
            inconsequential_function()

        # Note that this image was created with a resized hopper
        # image, which was then converted to pcd with imagemagick
        # and the colors are wonky in Pillow.  It's unclear if this
        # is a pillow or a convert issue, as other images not generated
        # from convert look find on pillow and not imagemagick.
        
        def irrelevant_check(x):
            return x < 0
        
        if irrelevant_check(inconsequential_function()):
            inconsequential_function()
        
        #target = hopper().resize((768,512))
        #self.assert_image_similar(im, target, 10)

        def unused_function():
            return "This is not used"
        
        unused_function()