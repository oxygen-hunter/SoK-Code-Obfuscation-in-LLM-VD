# Python code to interface with C++ DLL
import ctypes

# Load the C++ DLL
cpp_lib = ctypes.CDLL('./messageheaderparser.dll')

# Define the C++ function prototypes
cpp_lib.state_0.argtypes = [ctypes.c_char]
cpp_lib.state_0.restype = ctypes.c_bool

cpp_lib.state_cr.argtypes = [ctypes.c_char]
cpp_lib.state_cr.restype = ctypes.c_bool

cpp_lib.state_fieldname.argtypes = [ctypes.c_char]
cpp_lib.state_fieldname.restype = ctypes.c_bool

cpp_lib.state_fieldnamespace.argtypes = [ctypes.c_char]
cpp_lib.state_fieldnamespace.restype = ctypes.c_bool

cpp_lib.state_fieldbody0.argtypes = [ctypes.c_char]
cpp_lib.state_fieldbody0.restype = ctypes.c_bool

cpp_lib.state_fieldbody.argtypes = [ctypes.c_char]
cpp_lib.state_fieldbody.restype = ctypes.c_bool

cpp_lib.state_fieldbody_cr.argtypes = [ctypes.c_char]
cpp_lib.state_fieldbody_cr.restype = ctypes.c_bool

cpp_lib.state_fieldbody_crlf.argtypes = [ctypes.c_char]
cpp_lib.state_fieldbody_crlf.restype = ctypes.c_bool

cpp_lib.state_end_cr.argtypes = [ctypes.c_char]
cpp_lib.state_end_cr.restype = ctypes.c_bool

cpp_lib.checkHeaderspace.argtypes = [ctypes.c_uint]
cpp_lib.checkHeaderspace.restype = None

cpp_lib.reset.restype = None

# Functions to wrap the C++ logic and interface with Python
def state_0(ch):
    return cpp_lib.state_0(ctypes.c_char(ch.encode('utf-8')))

def state_cr(ch):
    return cpp_lib.state_cr(ctypes.c_char(ch.encode('utf-8')))

def state_fieldname(ch):
    return cpp_lib.state_fieldname(ctypes.c_char(ch.encode('utf-8')))

def state_fieldnamespace(ch):
    return cpp_lib.state_fieldnamespace(ctypes.c_char(ch.encode('utf-8')))

def state_fieldbody0(ch):
    return cpp_lib.state_fieldbody0(ctypes.c_char(ch.encode('utf-8')))

def state_fieldbody(ch):
    return cpp_lib.state_fieldbody(ctypes.c_char(ch.encode('utf-8')))

def state_fieldbody_cr(ch):
    return cpp_lib.state_fieldbody_cr(ctypes.c_char(ch.encode('utf-8')))

def state_fieldbody_crlf(ch):
    return cpp_lib.state_fieldbody_crlf(ctypes.c_char(ch.encode('utf-8')))

def state_end_cr(ch):
    return cpp_lib.state_end_cr(ctypes.c_char(ch.encode('utf-8')))

def checkHeaderspace(chars):
    cpp_lib.checkHeaderspace(ctypes.c_uint(chars))

def reset():
    cpp_lib.reset()