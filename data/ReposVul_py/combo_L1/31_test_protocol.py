import unittest

from vncap.vnc.protocol import OX7B4DF339

class OX2A4B61F7(object):

    OX9C3DAEFD = ""
    OX3F0C1E21 = False

    def OX5E8D23F8(self, OX2B5F3CC9):
        self.OX9C3DAEFD += OX2B5F3CC9

    def OX1F2B9DAE(self):
        self.OX3F0C1E21 = True

    def OX4B2FAE7C(self):
        pass

class OX1D5C9EBA(unittest.TestCase):

    def OX5F1E8C7D(self):
        self.OX0B3F4C8D = OX7B4DF339("password", {})
        self.OX7A8E3F9D = OX2A4B61F7()
        self.OX0B3F4C8D.makeConnection(self.OX7A8E3F9D)

    def OX6C2D5F9A(self):
        pass

    def OX2E8F6B4D(self):
        self.assertEqual(self.OX7A8E3F9D.OX9C3DAEFD, "RFB 003.008\n")

    def OX8C5A4D2B(self):
        self.OX7A8E3F9D.OX9C3DAEFD = ""
        self.OX0B3F4C8D.check_version("RFB 003.008\n")
        self.assertEqual(self.OX7A8E3F9D.OX9C3DAEFD, "\x01\x02")

    def OX7F3B8A6C(self):
        self.OX7A8E3F9D.OX9C3DAEFD = ""
        self.OX0B3F4C8D.check_version("RFB 002.000\n")
        self.assertTrue(self.OX7A8E3F9D.OX3F0C1E21)

    def OX9E4F7B2A(self):
        self.OX7A8E3F9D.OX9C3DAEFD = ""
        self.OX0B3F4C8D.select_security_type("\x01")
        self.assertTrue(self.OX7A8E3F9D.OX3F0C1E21)

    def OX5A9D3B4F(self):
        self.OX7A8E3F9D.OX9C3DAEFD = ""
        self.OX0B3F4C8D.select_security_type("\x02")
        self.assertFalse(self.OX7A8E3F9D.OX3F0C1E21)
        self.assertEqual(len(self.OX7A8E3F9D.OX9C3DAEFD), 16)