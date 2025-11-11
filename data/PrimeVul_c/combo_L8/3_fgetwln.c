import ctypes
import os

# Load the C standard library
libc = ctypes.CDLL(None)

class FILE(ctypes.Structure):
    pass

FILE_p = ctypes.POINTER(FILE)

# Define the C functions we are going to use
fgetwc = libc.fgetwc
fgetwc.argtypes = [FILE_p]
fgetwc.restype = ctypes.c_wint_t

reallocarray = libc.realloc
reallocarray.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
reallocarray.restype = ctypes.c_void_p

# Define the filewbuf structure in Python
class FileWBuf(ctypes.Structure):
    _fields_ = [("fp", FILE_p),
                ("wbuf", ctypes.POINTER(ctypes.c_wchar)),
                ("len", ctypes.c_size_t)]

FILEWBUF_INIT_LEN = 128
FILEWBUF_POOL_ITEMS = 32

fb_pool = (FileWBuf * FILEWBUF_POOL_ITEMS)()
fb_pool_cur = ctypes.c_int(0)

def fgetwln(stream, lenp):
    global fb_pool_cur
    fb = ctypes.pointer(fb_pool[fb_pool_cur.value])
    wused = 0

    if fb.contents.fp != stream and fb.contents.fp:
        fb_pool_cur.value += 1
        fb_pool_cur.value %= FILEWBUF_POOL_ITEMS
        fb = ctypes.pointer(fb_pool[fb_pool_cur.value])
    
    fb.contents.fp = stream

    while True:
        wc = fgetwc(stream)
        if wc == ctypes.c_wint_t(-1).value:
            break
        
        if not fb.contents.len or wused >= fb.contents.len:
            if fb.contents.len:
                fb.contents.len *= 2
            else:
                fb.contents.len = FILEWBUF_INIT_LEN
            
            wp = ctypes.cast(reallocarray(fb.contents.wbuf, fb.contents.len * ctypes.sizeof(ctypes.c_wchar)), ctypes.POINTER(ctypes.c_wchar))
            if not wp:
                wused = 0
                break
            fb.contents.wbuf = wp

        fb.contents.wbuf[wused] = wc
        wused += 1

        if wc == ord('\n'):
            break

    lenp[0] = wused
    return fb.contents.wbuf if wused else None