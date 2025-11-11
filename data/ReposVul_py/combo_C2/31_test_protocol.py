import unittest

from vncap.vnc.protocol import VNCServerAuthenticator

class DummyTransport(object):

    buf = ""
    lost = False

    def write(self, data):
        self.buf += data

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
        state = 0
        while True:
            if state == 0:
                self.assertEqual(self.t.buf, "RFB 003.008\n")
                state = 1
            elif state == 1:
                break

    def test_check_version(self):
        state = 0
        while True:
            if state == 0:
                self.t.buf = ""
                state = 1
            elif state == 1:
                self.p.check_version("RFB 003.008\n")
                state = 2
            elif state == 2:
                self.assertEqual(self.t.buf, "\x01\x02")
                state = 3
            elif state == 3:
                break

    def test_check_invalid_version(self):
        state = 0
        while True:
            if state == 0:
                self.t.buf = ""
                state = 1
            elif state == 1:
                self.p.check_version("RFB 002.000\n")
                state = 2
            elif state == 2:
                self.assertTrue(self.t.lost)
                state = 3
            elif state == 3:
                break

    def test_select_security_type_none(self):
        state = 0
        while True:
            if state == 0:
                self.t.buf = ""
                state = 1
            elif state == 1:
                self.p.select_security_type("\x01")
                state = 2
            elif state == 2:
                self.assertTrue(self.t.lost)
                state = 3
            elif state == 3:
                break

    def test_select_security_type_vnc_auth(self):
        state = 0
        while True:
            if state == 0:
                self.t.buf = ""
                state = 1
            elif state == 1:
                self.p.select_security_type("\x02")
                state = 2
            elif state == 2:
                self.assertFalse(self.t.lost)
                state = 3
            elif state == 3:
                self.assertEqual(len(self.t.buf), 16)
                state = 4
            elif state == 4:
                break