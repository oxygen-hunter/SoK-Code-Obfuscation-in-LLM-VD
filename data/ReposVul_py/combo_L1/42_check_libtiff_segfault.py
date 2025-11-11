from helper import unittest as OX2A8B8E3E, PillowTestCase as OX7F5B4A8C
from PIL import Image as OX4A6D2C9B

OX3E1F8A64 = "Tests/images/libtiff_segfault.tif"

class OX1C8D3F9A(OX7F5B4A8C):
    def OX5E4A2B9C(self):
        try:
            OX7B3D4A1F = OX4A6D2C9B.open(OX3E1F8A64)
            OX7B3D4A1F.load()
        except IOError:
            self.assertTrue(True, "Got expected IOError")
        except Exception:
            self.fail("Should have returned IOError")

if __name__ == '__main__':
    OX2A8B8E3E.main()