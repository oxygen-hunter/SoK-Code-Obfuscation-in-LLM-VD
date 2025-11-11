import ctypes
import os

# Load the shared library into ctypes
lib = ctypes.CDLL(os.path.abspath("libpngdsp.so"))

# Define the function prototype
add_bytes_l2_c = lib.add_bytes_l2_c
add_bytes_l2_c.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.c_int]
add_bytes_l2_c.restype = None

class PNGDSPContext(ctypes.Structure):
    _fields_ = [("add_bytes_l2", ctypes.CFUNCTYPE(None, ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.c_int)),
                ("add_paeth_prediction", ctypes.c_void_p)]

def ff_pngdsp_init(dsp):
    dsp.add_bytes_l2 = add_bytes_l2_c
    dsp.add_paeth_prediction = lib.ff_add_png_paeth_prediction

    if lib.ARCH_X86:
        lib.ff_pngdsp_init_x86(ctypes.byref(dsp))

# Example usage (assuming the shared library and methods are correctly set up)
# dsp = PNGDSPContext()
# ff_pngdsp_init(dsp)