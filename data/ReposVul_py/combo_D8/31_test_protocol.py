import unittest

from vncap.vnc.protocol import VNCServerAuthenticator

class DummyTransport(object):

    def __init__(self):
        self.dynamic_buf = ""
        self.dynamic_lost = False

    def get_buffer(self):
        return self.dynamic_buf

    def set_buffer(self, value):
        self.dynamic_buf = value

    def is_lost(self):
        return self.dynamic_lost

    def set_lost(self, value):
        self.dynamic_lost = value

    def write(self, data):
        self.set_buffer(self.get_buffer() + data)

    def loseConnection(self):
        self.set_lost(True)

    def pauseProducing(self):
        pass

class TestVNCServerAuthenticator(unittest.TestCase):

    def setUp(self):
        self.p = VNCServerAuthenticator("password", {})
        self.t = DummyTransport()
        self.p.makeConnection(self.t)

    def test_trivial(self):
        pass

    def test_connectionMade(self):
        self.assertEqual(self.t.get_buffer(), "RFB 003.008\n")

    def test_check_version(self):
        self.t.set_buffer("")
        self.p.check_version("RFB 003.008\n")
        self.assertEqual(self.t.get_buffer(), "\x01\x02")

    def test_check_invalid_version(self):
        self.t.set_buffer("")
        self.p.check_version("RFB 002.000\n")
        self.assertTrue(self.t.is_lost())

    def test_select_security_type_none(self):
        self.t.set_buffer("")
        self.p.select_security_type("\x01")
        self.assertTrue(self.t.is_lost())

    def test_select_security_type_vnc_auth(self):
        self.t.set_buffer("")
        self.p.select_security_type("\x02")
        self.assertFalse(self.t.is_lost())
        self.assertEqual(len(self.t.get_buffer()), 16)