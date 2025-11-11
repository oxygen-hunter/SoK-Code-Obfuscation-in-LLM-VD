from helper import unittest,PillowTestCase,hopper
from PIL import Image
class TestFilePcd(PillowTestCase): 
 def test_load_raw(self):im=Image.open('Tests/images/hopper.pcd');im.load()
if __name__=='__main__':unittest.main()