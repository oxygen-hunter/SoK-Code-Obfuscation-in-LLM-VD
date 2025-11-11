# Import necessary libraries
import ctypes
from ctypes import c_uint32, c_char_p, c_bool, POINTER, byref, create_string_buffer

# Load C DLL
lib = ctypes.CDLL('./libgssapi.so')

# Define C function prototypes
lib.xdr_bytes.argtypes = [POINTER(c_uint32), POINTER(c_char_p), POINTER(c_uint32), c_uint32]
lib.xdr_bytes.restype = c_bool

lib.gss_seal.argtypes = [POINTER(c_uint32), c_uint32, c_uint32, c_uint32, POINTER(c_uint32), POINTER(c_uint32), POINTER(c_uint32)]
lib.gss_seal.restype = c_uint32

lib.gss_unseal.argtypes = [POINTER(c_uint32), c_uint32, POINTER(c_uint32), POINTER(c_uint32), POINTER(c_uint32), POINTER(c_uint32)]
lib.gss_unseal.restype = c_uint32

def xdr_gss_buf(xdrs, buf):
    length = c_uint32(buf.length)
    result = lib.xdr_bytes(byref(xdrs), byref(c_char_p(buf.value)), byref(length), (xdrs.x_op == 1 and buf.value is None) and c_uint32(-1) or c_uint32(buf.length))
    buf.length = length.value
    return result

def auth_gssapi_seal_seq(context, seq_num, out_buf):
    nl_seq_num = c_uint32(seq_num).value.to_bytes(4, 'big')
    in_buf = create_string_buffer(nl_seq_num, 4)
    gssstat = lib.gss_seal(byref(c_uint32()), context, 0, 0, byref(in_buf), None, byref(out_buf))
    if gssstat != 0:
        return False
    return True

def auth_gssapi_unseal_seq(context, in_buf, seq_num):
    out_buf = create_string_buffer(4)
    gssstat = lib.gss_unseal(byref(c_uint32()), context, byref(in_buf), byref(out_buf), None, None)
    if gssstat != 0 or len(out_buf) != 4:
        return False
    seq_num.value = int.from_bytes(out_buf, 'big')
    return True