# Import necessary modules
from ctypes import CDLL, Structure, POINTER, c_void_p

# Load the C library
lib = CDLL('./sctp_c_lib.so')

# Define necessary structs
class SCTP_INQ(Structure):
    _fields_ = [("in_chunk_list", c_void_p),
                ("in_progress", c_void_p),
                ("immediate", c_void_p)]

class SCTP_CHUNK(Structure):
    _fields_ = [("rcvr", c_void_p),
                ("asoc", c_void_p),
                ("list", c_void_p)]

# Define function prototypes
lib.sctp_inq_init.argtypes = [POINTER(SCTP_INQ)]
lib.sctp_inq_free.argtypes = [POINTER(SCTP_INQ)]
lib.sctp_inq_push.argtypes = [POINTER(SCTP_INQ), POINTER(SCTP_CHUNK)]
lib.sctp_inq_peek.argtypes = [POINTER(SCTP_INQ)]
lib.sctp_inq_pop.argtypes = [POINTER(SCTP_INQ)]
lib.sctp_inq_set_th_handler.argtypes = [POINTER(SCTP_INQ), c_void_p]

# Python wrapper functions
def py_sctp_inq_init(queue):
    lib.sctp_inq_init(queue)

def py_sctp_inq_free(queue):
    lib.sctp_inq_free(queue)

def py_sctp_inq_push(queue, chunk):
    lib.sctp_inq_push(queue, chunk)

def py_sctp_inq_peek(queue):
    return lib.sctp_inq_peek(queue)

def py_sctp_inq_pop(queue):
    return lib.sctp_inq_pop(queue)

def py_sctp_inq_set_th_handler(queue, callback):
    lib.sctp_inq_set_th_handler(queue, callback)