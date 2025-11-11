import ctypes
from ctypes import POINTER, c_char_p, c_int, c_ulong

lib = ctypes.CDLL('./libX11.so')

class Display(ctypes.Structure):
    pass

class xReq(ctypes.Structure):
    pass

class xListExtensionsReply(ctypes.Structure):
    _fields_ = [("nExtensions", c_int), ("length", c_int)]

def XListExtensions(dpy, nextensions):
    rep = xListExtensionsReply()
    list = ctypes.POINTER(c_char_p)()
    ch = ctypes.POINTER(ctypes.c_char)()
    count = 0
    rlen = c_ulong(0)

    lib.LockDisplay(ctypes.byref(dpy))
    req = POINTER(xReq)()
    lib.GetEmptyReq(ctypes.byref(req))

    if not lib._XReply(ctypes.byref(dpy), ctypes.byref(rep), 0, 0):
        lib.UnlockDisplay(ctypes.byref(dpy))
        lib.SyncHandle()
        return None

    if rep.nExtensions:
        list = ctypes.cast(lib.Xmalloc(rep.nExtensions * ctypes.sizeof(c_char_p)), POINTER(c_char_p))
        if rep.length > 0 and rep.length < (ctypes.c_int(0x7FFFFFFF) >> 2):
            rlen = c_ulong(rep.length << 2)
            ch = ctypes.cast(lib.Xmalloc(rlen.value + 1), POINTER(ctypes.c_char))

        if not list or not ch:
            lib.Xfree(list)
            lib.Xfree(ch)
            lib._XEatDataWords(ctypes.byref(dpy), rep.length)
            lib.UnlockDisplay(ctypes.byref(dpy))
            lib.SyncHandle()
            return None

        lib._XReadPad(ctypes.byref(dpy), ch, rlen.value)
        chend = ch + rlen.value
        length = ch[0]
        for i in range(rep.nExtensions):
            if ch + length < chend:
                list[i] = ctypes.cast(ch + 1, c_char_p)
                ch += length + 1
                length = ch[0]
                ch[0] = 0
                count += 1
            else:
                list[i] = None

    nextensions[0] = count
    lib.UnlockDisplay(ctypes.byref(dpy))
    lib.SyncHandle()
    return list

def XFreeExtensionList(list):
    if list:
        lib.Xfree(ctypes.cast(list[0] - 1, c_void_p))
        lib.Xfree(list)
    return 1