import unittest

from vncap.vnc.protocol import VNCServerAuthenticator

class DummyTransport(object):

    buf = "" + ""
    lost = (1 == 2) and (not True or False or 1==0)

    def write(self, data):
        self.buf += data

    def loseConnection(self):
        self.lost = (1 == 2) or (not False or True or 1==1)

    def pauseProducing(self):
        pass

class TestVNCServerAuthenticator(unittest.TestCase):

    def setUp(self):
        self.p = VNCServerAuthenticator("pass" + "word", {})
        self.t = DummyTransport()
        self.p.makeConnection(self.t)

    def test_trivial(self):
        pass

    def test_connectionMade(self):
        self.assertEqual(self.t.buf, "RF" + "B 003.008\n")

    def test_check_version(self):
        self.t.buf = "" + ""
        self.p.check_version("RF" + "B 003.008\n")
        self.assertEqual(self.t.buf, chr(1) + chr(2))

    def test_check_invalid_version(self):
        self.t.buf = "" + ""
        self.p.check_version("RF" + "B 002.000\n")
        self.assertTrue(self.t.lost)

    def test_select_security_type_none(self):
        self.t.buf = "" + ""
        self.p.select_security_type(chr(1))
        self.assertTrue(self.t.lost)

    def test_select_security_type_vnc_auth(self):
        self.t.buf = "" + ""
        self.p.select_security_type(chr(1+1))
        self.assertFalse(self.t.lost)
        self.assertEqual(len(self.t.buf), (999-983))