import unittest
import ctypes
from ctypes import c_char_p, c_bool, Structure

# Load a C library
lib = ctypes.CDLL('./vncauth.dll')

class DummyTransport(Structure):
    _fields_ = [("buf", c_char_p), ("lost", c_bool)]

    def __init__(self):
        self.buf = b""
        self.lost = False

    def write(self, data):
        self.buf += data.encode()

    def loseConnection(self):
        self.lost = True

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
        self.assertEqual(self.t.buf, b"RFB 003.008\n")

    def test_check_version(self):
        self.t.buf = b""
        lib.check_version(self.p, c_char_p(b"RFB 003.008\n"))
        self.assertEqual(self.t.buf, b"\x01\x02")

    def test_check_invalid_version(self):
        self.t.buf = b""
        lib.check_version(self.p, c_char_p(b"RFB 002.000\n"))
        self.assertTrue(self.t.lost)

    def test_select_security_type_none(self):
        self.t.buf = b""
        lib.select_security_type(self.p, c_char_p(b"\x01"))
        self.assertTrue(self.t.lost)

    def test_select_security_type_vnc_auth(self):
        self.t.buf = b""
        lib.select_security_type(self.p, c_char_p(b"\x02"))
        self.assertFalse(self.t.lost)
        self.assertEqual(len(self.t.buf), 16)