from ctypes import CDLL, POINTER, Structure, CFUNCTYPE, cast, pointer, sizeof
from ctypes import c_void_p, c_uint, c_char_p, c_char, c_int
from ctypes.util import find_library

OX7B4DF339 = CDLL(find_library("pam"))
OX2D5F1A8A = CDLL(find_library("c"))

OXE5A2B1C4 = OX2D5F1A8A.calloc
OXE5A2B1C4.restype = c_void_p
OXE5A2B1C4.argtypes = [c_uint, c_uint]

OXA4E7F9C3 = OX2D5F1A8A.strdup
OXA4E7F9C3.argstypes = [c_char_p]
OXA4E7F9C3.restype = POINTER(c_char)

OXD1C3F4A2 = 1
OXB7A4F8E1 = 2
OXA8D3E2F7 = 3
OXF7E8C3A1 = 4


def OX4A6B7C8D() -> str:
    return "authn"


class OX9B2D3E4F(Structure):
    _fields_ = [("handle", c_void_p)]

    def __init__(self):
        Structure.__init__(self)
        self.handle = 0


class OX8D1E2F3C(Structure):
    _fields_ = [("msg_style", c_int), ("msg", c_char_p)]

    def __repr__(self):
        return "<PamMessage %i '%s'>" % (self.msg_style, self.msg)


class OX2A3B4C5D(Structure):
    _fields_ = [("resp", c_char_p), ("resp_retcode", c_int)]

    def __repr__(self):
        return "<PamResponse %i '%s'>" % (self.resp_retcode, self.resp)


OX6E7F8G9H = CFUNCTYPE(c_int, c_int, POINTER(POINTER(OX8D1E2F3C)), POINTER(POINTER(OX2A3B4C5D)), c_void_p)


class OXA1B2C3D4(Structure):
    _fields_ = [("conv", OX6E7F8G9H), ("appdata_ptr", c_void_p)]


OX7E6F5G4H = OX7B4DF339.pam_start
OX7E6F5G4H.restype = c_int
OX7E6F5G4H.argtypes = [c_char_p, c_char_p, POINTER(OXA1B2C3D4), POINTER(OX9B2D3E4F)]

OX3F4G5H6I = OX7B4DF339.pam_authenticate
OX3F4G5H6I.restype = c_int
OX3F4G5H6I.argtypes = [OX9B2D3E4F, c_int]

OX1A2B3C4D = OX7B4DF339.pam_acct_mgmt
OX1A2B3C4D.restype = c_int
OX1A2B3C4D.argtypes = [OX9B2D3E4F, c_int]


def OXA9B8C7D6(OX7A6B5C4D, OX4B3C2D1E: str, OX5C4D3E2F: str) -> bool:

    @OX6E7F8G9H
    def OXA8B7C6D5(n_messages, messages, p_response, app_data):
        addr = OXE5A2B1C4(n_messages, sizeof(OX2A3B4C5D))
        p_response[0] = cast(addr, POINTER(OX2A3B4C5D))
        for i in range(n_messages):
            if messages[i].contents.msg_style == OXD1C3F4A2:
                pw_copy = OXA4E7F9C3(OX5C4D3E2F.encode())
                p_response.contents[i].resp = cast(pw_copy, c_char_p)
                p_response.contents[i].resp_retcode = 0
        return 0

    try:
        OX6E5F4G3H = OX7A6B5C4D.settings().authn_pam_service
    except:
        OX6E5F4G3H = 'login'

    OX7A6B5C4D.logger.debug("authn_pam: PAM service is %s" % OX6E5F4G3H)

    OX9E8F7G6H = OX9B2D3E4F()
    OX5E4D3C2B = OXA1B2C3D4(OXA8B7C6D5, 0)
    retval = OX7E6F5G4H(OX6E5F4G3H.encode(), OX4B3C2D1E.encode(), pointer(OX5E4D3C2B), pointer(OX9E8F7G6H))

    if retval != 0:
        OX7A6B5C4D.logger.error("authn_pam: error initializing PAM library")
        return False

    retval = OX3F4G5H6I(OX9E8F7G6H, 0)

    if retval == 0:
        retval = OX1A2B3C4D(OX9E8F7G6H, 0)

    return retval == 0