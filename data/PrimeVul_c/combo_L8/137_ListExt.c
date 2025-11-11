import ctypes
from ctypes import c_int, c_char_p, POINTER, Structure, c_ulong, c_void_p

lib = ctypes.CDLL('./libX11.so')

class Display(Structure):
    pass

class xReply(Structure):
    pass

class xListExtensionsReply(Structure):
    _fields_ = [
        ('nExtensions', c_int),
        ('length', c_ulong)
    ]

lib.LockDisplay.argtypes = [POINTER(Display)]
lib.LockDisplay.restype = None

lib.UnlockDisplay.argtypes = [POINTER(Display)]
lib.UnlockDisplay.restype = None

lib.GetEmptyReq.argtypes = [c_int, c_void_p]
lib.GetEmptyReq.restype = None

lib._XReply.argtypes = [POINTER(Display), POINTER(xReply), c_int, c_int]
lib._XReply.restype = c_int

lib.Xmalloc.argtypes = [c_ulong]
lib.Xmalloc.restype = c_void_p

lib.Xfree.argtypes = [c_void_p]
lib.Xfree.restype = None

lib._XReadPad.argtypes = [POINTER(Display), c_void_p, c_ulong]
lib._XReadPad.restype = None

lib._XEatDataWords.argtypes = [POINTER(Display), c_ulong]
lib._XEatDataWords.restype = None

lib.SyncHandle.restype = None

def XListExtensions(dpy, nextensions):
    rep = xListExtensionsReply()
    list = None
    ch = None
    count = 0
    rlen = c_ulong(0)

    lib.LockDisplay(dpy)
    req = c_void_p()
    lib.GetEmptyReq(0, ctypes.byref(req))

    if not lib._XReply(dpy, ctypes.byref(rep), 0, 0):
        lib.UnlockDisplay(dpy)
        lib.SyncHandle()
        return None

    if rep.nExtensions:
        list = ctypes.cast(lib.Xmalloc(rep.nExtensions * ctypes.sizeof(c_char_p)), POINTER(c_char_p))
        if rep.length > 0 and rep.length < (ctypes.c_uint(-1).value >> 2):
            rlen.value = rep.length << 2
            ch = ctypes.cast(lib.Xmalloc(rlen.value + 1), POINTER(ctypes.c_char))

        if not list or not ch:
            lib.Xfree(list)
            lib.Xfree(ch)
            lib._XEatDataWords(dpy, rep.length)
            lib.UnlockDisplay(dpy)
            lib.SyncHandle()
            return None

        lib._XReadPad(dpy, ch, rlen.value)
        chend = ctypes.cast(ctypes.addressof(ch.contents) + rlen.value + 1, POINTER(ctypes.c_char))
        length = ch[0]
        i = 0
        while i < rep.nExtensions:
            if ctypes.addressof(ch.contents) + length < ctypes.addressof(chend.contents):
                list[i] = ctypes.cast(ctypes.addressof(ch.contents) + 1, c_char_p)
                ch = ctypes.cast(ctypes.addressof(ch.contents) + length + 1, POINTER(ctypes.c_char))
                if ctypes.addressof(ch.contents) <= ctypes.addressof(chend.contents):
                    length = ch[0]
                    ch[0] = 0
                    count += 1
                else:
                    list[i] = None
            else:
                list[i] = None
            i += 1

    nextensions[0] = count
    lib.UnlockDisplay(dpy)
    lib.SyncHandle()
    return list

def XFreeExtensionList(list):
    if list:
        lib.Xfree(ctypes.cast(list[0], POINTER(c_char)) - 1)
        lib.Xfree(list)
    return 1