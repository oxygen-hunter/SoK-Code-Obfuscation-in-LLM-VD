from ctypes import CDLL, POINTER, Structure, CFUNCTYPE, cast, pointer, sizeof
from ctypes import c_void_p, c_uint, c_char_p, c_char, c_int
from ctypes.util import find_library

LIBPAM = CDLL(find_library("pam"))
LIBC = CDLL(find_library("c"))

CALLOC = LIBC.calloc
CALLOC.restype = c_void_p
CALLOC.argtypes = [c_uint, c_uint]

STRDUP = LIBC.strdup
STRDUP.argstypes = [c_char_p]
STRDUP.restype = POINTER(c_char)

PAM_PROMPT_ECHO_OFF = 1
PAM_PROMPT_ECHO_ON = 2
PAM_ERROR_MSG = 3
PAM_TEXT_INFO = 4

class VM:
    def __init__(self):
        self.stack = []
        self.program_counter = 0
        self.instructions = []

    def execute(self, instructions):
        self.program_counter = 0
        self.instructions = instructions
        while self.program_counter < len(self.instructions):
            opcode, *args = self.instructions[self.program_counter]
            self.program_counter += 1
            if opcode == "PUSH":
                self.stack.append(args[0])
            elif opcode == "POP":
                self.stack.pop()
            elif opcode == "CALL":
                self.stack.append(args[0](*self.stack))
            elif opcode == "RET":
                return self.stack.pop()
            elif opcode == "JMP":
                self.program_counter = args[0]
            elif opcode == "JZ":
                if not self.stack.pop():
                    self.program_counter = args[0]
            elif opcode == "LOAD":
                self.stack.append(args[0])
            elif opcode == "STORE":
                self.stack[args[0]] = self.stack.pop()
            elif opcode == "ADD":
                self.stack.append(self.stack.pop() + self.stack.pop())
            elif opcode == "SUB":
                self.stack.append(self.stack.pop() - self.stack.pop())
            elif opcode == "DEBUG":
                print("DEBUG:", self.stack)

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

PAM_START = LIBPAM.pam_start
PAM_START.restype = c_int
PAM_START.argtypes = [c_char_p, c_char_p, POINTER(PamConv), POINTER(PamHandle)]

PAM_AUTHENTICATE = LIBPAM.pam_authenticate
PAM_AUTHENTICATE.restype = c_int
PAM_AUTHENTICATE.argtypes = [PamHandle, c_int]

PAM_ACCT_MGMT = LIBPAM.pam_acct_mgmt
PAM_ACCT_MGMT.restype = c_int
PAM_ACCT_MGMT.argtypes = [PamHandle, c_int]

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

    instructions = [
        ("LOAD", api_handle),
        ("CALL", lambda api: api.settings().authn_pam_service if hasattr(api, 'settings') else 'login'),
        ("STORE", 0),
        ("LOAD", api_handle),
        ("DEBUG",),
        ("CALL", lambda api: api.logger.debug("authn_pam: PAM service is %s" % api.settings().authn_pam_service)),
        ("LOAD", PamHandle()),
        ("LOAD", PamConv(my_conv, 0)),
        ("LOAD", lambda: pointer(PamHandle())),
        ("LOAD", lambda: pointer(PamConv(my_conv, 0))),
        ("LOAD", lambda: PAM_START),
        ("CALL", lambda pam_start_fn, service, username, conv_ptr, handle_ptr: pam_start_fn(service.encode(), username.encode(), pointer(conv_ptr), pointer(handle_ptr))),
        ("STORE", 1),
        ("LOAD", lambda: 0),
        ("LOAD", lambda: api_handle.logger.error("authn_pam: error initializing PAM library")),
        ("JZ", 25),
        ("LOAD", lambda: PAM_AUTHENTICATE),
        ("LOAD", lambda handle: handle),
        ("LOAD", lambda: 0),
        ("CALL", lambda pam_auth_fn, handle, flags: pam_auth_fn(handle, flags)),
        ("STORE", 2),
        ("LOAD", lambda: 0),
        ("JZ", 21),
        ("LOAD", lambda: PAM_ACCT_MGMT),
        ("LOAD", lambda handle: handle),
        ("LOAD", lambda: 0),
        ("CALL", lambda pam_acct_fn, handle, flags: pam_acct_fn(handle, flags)),
        ("STORE", 2),
        ("LOAD", lambda: 0),
        ("SUB",),
        ("RET",),
    ]

    vm = VM()
    result = vm.execute(instructions)
    return result == 0