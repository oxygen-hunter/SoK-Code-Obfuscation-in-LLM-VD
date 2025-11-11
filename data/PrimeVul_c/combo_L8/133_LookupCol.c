import ctypes
import ctypes.util

# Load the shared library
x11 = ctypes.CDLL(ctypes.util.find_library('X11'))

# Define the structures and constants
class Display(ctypes.Structure):
    pass

class XColor(ctypes.Structure):
    _fields_ = [("red", ctypes.c_ushort),
                ("green", ctypes.c_ushort),
                ("blue", ctypes.c_ushort),
                ("flags", ctypes.c_char),
                ("pad", ctypes.c_char)]

class xLookupColorReply(ctypes.Structure):
    _fields_ = [("type", ctypes.c_ubyte),
                ("pad1", ctypes.c_ubyte),
                ("sequenceNumber", ctypes.c_ushort),
                ("length", ctypes.c_uint),
                ("exactRed", ctypes.c_ushort),
                ("exactGreen", ctypes.c_ushort),
                ("exactBlue", ctypes.c_ushort),
                ("screenRed", ctypes.c_ushort),
                ("screenGreen", ctypes.c_ushort),
                ("screenBlue", ctypes.c_ushort),
                ("pad2", ctypes.c_ushort)]

class xLookupColorReq(ctypes.Structure):
    _fields_ = [("reqType", ctypes.c_ubyte),
                ("pad", ctypes.c_ubyte),
                ("length", ctypes.c_ushort),
                ("cmap", ctypes.c_uint),
                ("nbytes", ctypes.c_ushort)]

Status = ctypes.c_int
Colormap = ctypes.c_uint
_Xconst_char_p = ctypes.c_char_p

def XLookupColor(dpy, cmap, spec, def_color, scr):
    n = len(spec)
    x11.XLockDisplay(dpy)
    req = xLookupColorReq()
    req.cmap = cmap
    req.nbytes = n
    req.length += (n + 3) >> 2
    x11.Data(ctypes.byref(dpy), spec.encode('utf-8'), ctypes.c_long(n))
    reply = xLookupColorReply()
    if not x11._XReply(dpy, ctypes.byref(reply), 0, True):
        x11.XUnlockDisplay(dpy)
        x11.XSyncHandle()
        return 0

    def_color.red = reply.exactRed
    def_color.green = reply.exactGreen
    def_color.blue = reply.exactBlue

    scr.red = reply.screenRed
    scr.green = reply.screenGreen
    scr.blue = reply.screenBlue

    x11.XUnlockDisplay(dpy)
    x11.XSyncHandle()
    return 1