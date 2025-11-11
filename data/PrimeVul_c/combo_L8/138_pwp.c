import ctypes
from ctypes import c_char_p, c_size_t, c_int, c_void_p, c_ulong, POINTER

# Load the ImageMagick C library
imlib = ctypes.CDLL('libMagickCore-7.Q16HDRI.so')

# Define Python wrappers for the necessary C functions
IsPWP_c = imlib.IsPWP
IsPWP_c.argtypes = [POINTER(ctypes.c_ubyte), c_size_t]
IsPWP_c.restype = ctypes.c_int

ReadPWPImage_c = imlib.ReadPWPImage
ReadPWPImage_c.argtypes = [c_void_p, c_void_p]
ReadPWPImage_c.restype = c_void_p

RegisterPWPImage_c = imlib.RegisterPWPImage
RegisterPWPImage_c.argtypes = []
RegisterPWPImage_c.restype = c_ulong

UnregisterPWPImage_c = imlib.UnregisterPWPImage
UnregisterPWPImage_c.argtypes = []
UnregisterPWPImage_c.restype = None

def IsPWP(magick, length):
    magick_bytes = (ctypes.c_ubyte * length)(*magick)
    return IsPWP_c(magick_bytes, length)

def ReadPWPImage(image_info, exception):
    return ReadPWPImage_c(image_info, exception)

def RegisterPWPImage():
    return RegisterPWPImage_c()

def UnregisterPWPImage():
    UnregisterPWPImage_c()