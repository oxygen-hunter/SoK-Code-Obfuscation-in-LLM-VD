from ctypes import CDLL, POINTER, Structure, CFUNCTYPE, cast, pointer, sizeof
from ctypes import c_void_p, c_uint, c_char_p, c_char, c_int
from ctypes.util import find_library

def get_libpam():
    return CDLL(find_library("pam"))

def get_libc():
    return CDLL(find_library("c"))

def get_calloc(lib):
    func = lib.calloc
    func.restype = c_void_p
    func.argtypes = [c_uint, c_uint]
    return func

def get_strdup(lib):
    func = lib.strdup
    func.argstypes = [c_char_p]
    func.restype = POINTER(c_char)
    return func

def get_pam_prompt_echo_off():
    return 1

def get_pam_prompt_echo_on():
    return 2

def get_pam_error_msg():
    return 3

def get_pam_text_info():
    return 4

LIBPAM = get_libpam()
LIBC = get_libc()

CALLOC = get_calloc(LIBC)
STRDUP = get_strdup(LIBC)

PAM_PROMPT_ECHO_OFF = get_pam_prompt_echo_off()
PAM_PROMPT_ECHO_ON = get_pam_prompt_echo_on()
PAM_ERROR_MSG = get_pam_error_msg()
PAM_TEXT_INFO = get_pam_text_info()

def register() -> str:
    return "authn"

class PamHandle(Structure):
    _fields_ = [("handle", c_void_p)]

    def __init__(self):
        Structure.__init__(self)
        self.handle = 0

class PamMessage(Structure):
    _fields_ = [("msg_style", c_int), ("msg", c_char_p)]

    def __repr__(self):
        return "<PamMessage %i '%s'>" % (self.msg_style, self.msg)

class PamResponse(Structure):
    _fields_ = [("resp", c_char_p), ("resp_retcode", c_int)]

    def __repr__(self):
        return "<PamResponse %i '%s'>" % (self.resp_retcode, self.resp)

CONV_FUNC = CFUNCTYPE(c_int, c_int, POINTER(POINTER(PamMessage)), POINTER(POINTER(PamResponse)), c_void_p)

class PamConv(Structure):
    _fields_ = [("conv", CONV_FUNC), ("appdata_ptr", c_void_p)]

def get_pam_start(lib):
    func = lib.pam_start
    func.restype = c_int
    func.argtypes = [c_char_p, c_char_p, POINTER(PamConv), POINTER(PamHandle)]
    return func

def get_pam_authenticate(lib):
    func = lib.pam_authenticate
    func.restype = c_int
    func.argtypes = [PamHandle, c_int]
    return func

def get_pam_acct_mgmt(lib):
    func = lib.pam_acct_mgmt
    func.restype = c_int
    func.argtypes = [PamHandle, c_int]
    return func

PAM_START = get_pam_start(LIBPAM)
PAM_AUTHENTICATE = get_pam_authenticate(LIBPAM)
PAM_ACCT_MGMT = get_pam_acct_mgmt(LIBPAM)

def authenticate(api_handle, username: str, password: str) -> bool:
    @CONV_FUNC
    def my_conv(n_messages, messages, p_response, app_data):
        addr = CALLOC(n_messages, sizeof(PamResponse))
        p_response[0] = cast(addr, POINTER(PamResponse))
        for i in range(n_messages):
            if messages[i].contents.msg_style == PAM_PROMPT_ECHO_OFF:
                pw_copy = STRDUP(password.encode())
                p_response.contents[i].resp = cast(pw_copy, c_char_p)
                p_response.contents[i].resp_retcode = 0
        return 0

    try:
        service = api_handle.settings().authn_pam_service
    except:
        service = 'login'

    api_handle.logger.debug("authn_pam: PAM service is %s" % service)

    handle = PamHandle()
    conv = PamConv(my_conv, 0)
    retval = PAM_START(service.encode(), username.encode(), pointer(conv), pointer(handle))

    if retval != 0:
        api_handle.logger.error("authn_pam: error initializing PAM library")
        return False

    retval = PAM_AUTHENTICATE(handle, 0)

    if retval == 0:
        retval = PAM_ACCT_MGMT(handle, 0)

    return retval == 0