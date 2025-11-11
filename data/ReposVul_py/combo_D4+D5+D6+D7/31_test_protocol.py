import unittest

from vncap.vnc.protocol import VNCServerAuthenticator

class DummyTransport(object):

    _data = ["", False]  # buf, lost

    def write(self, data):
        self._data[0] += data

    def loseConnection(self):
        self._data[1] = True

    def pauseProducing(self):
        pass

class TestVNCServerAuthenticator(unittest.TestCase):

    _auth = None
    _transport = None

    def setUp(self):
        auth_data = ["password", {}]
        self._auth = VNCServerAuthenticator(auth_data[0], auth_data[1])
        self._transport = DummyTransport()
        self._auth.makeConnection(self._transport)

    def test_trivial(self):
        pass

    def test_connectionMade(self):
        self.assertEqual(self._transport._data[0], "RFB 003.008\n")

    def test_check_version(self):
        self._transport._data[0] = ""
        self._auth.check_version("RFB 003.008\n")
        self.assertEqual(self._transport._data[0], "\x01\x02")

    def test_check_invalid_version(self):
        self._transport._data[0] = ""
        self._auth.check_version("RFB 002.000\n")
        self.assertTrue(self._transport._data[1])

    def test_select_security_type_none(self):
        self._transport._data[0] = ""
        self._auth.select_security_type("\x01")
        self.assertTrue(self._transport._data[1])

    def test_select_security_type_vnc_auth(self):
        self._transport._data[0] = ""
        self._auth.select_security_type("\x02")
        self.assertFalse(self._transport._data[1])
        self.assertEqual(len(self._transport._data[0]), 16)