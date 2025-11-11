import ctypes
from ctypes import POINTER, byref, c_int, c_void_p

lib = ctypes.CDLL('./eventlib.so')  # Assuming eventlib.so is a compiled C library containing necessary functions

class Client(ctypes.Structure):
    pass

class xEvent(ctypes.Structure):
    _fields_ = [("u", c_int)]

class DeviceIntPtr(ctypes.Structure):
    pass

class XEventClass(ctypes.Structure):
    pass

def ProcXSendExtensionEvent(client):
    ret = c_int()
    dev = POINTER(DeviceIntPtr)()
    first = POINTER(xEvent)()
    list = POINTER(XEventClass)()
    tmp = (c_void_p * EMASKSIZE)()

    request_size = lib.bytes_to_int32(ctypes.sizeof(xSendExtensionEventReq)) + stuff.count + (stuff.num_events * lib.bytes_to_int32(ctypes.sizeof(xEvent)))
    if stuff.length != request_size:
        return lib.BadLength

    ret.value = lib.dixLookupDevice(byref(dev), stuff.deviceid, client, lib.DixWriteAccess)
    if ret.value != lib.Success:
        return ret.value

    if stuff.num_events == 0:
        return ret.value

    first = ctypes.cast(ctypes.byref(stuff, ctypes.sizeof(xSendExtensionEventReq)), POINTER(xEvent))
    if not ((lib.EXTENSION_EVENT_BASE <= first.contents.u.u.type) and (first.contents.u.u.type < lib.lastEvent)):
        client.errorValue = first.contents.u.u.type
        return lib.BadValue

    list = ctypes.cast(ctypes.byref(first, stuff.num_events * ctypes.sizeof(xEvent)), POINTER(XEventClass))
    if lib.CreateMaskFromList(client, list, stuff.count, tmp, dev, lib.X_SendExtensionEvent) != lib.Success:
        return ret.value

    ret.value = lib.SendEvent(client, dev, stuff.destination, stuff.propagate, ctypes.cast(ctypes.byref(stuff, ctypes.sizeof(xSendExtensionEventReq)), POINTER(xEvent)), tmp[stuff.deviceid], stuff.num_events)

    return ret.value

def SProcXSendExtensionEvent(client):
    p = POINTER(c_int)()
    i = c_int()
    eventT = xEvent()
    eventP = POINTER(xEvent)()
    proc = c_void_p()

    lib.REQUEST(xSendExtensionEventReq)
    lib.swaps(byref(stuff.length))
    lib.REQUEST_AT_LEAST_SIZE(xSendExtensionEventReq)
    lib.swapl(byref(stuff.destination))
    lib.swaps(byref(stuff.count))

    expected_length = lib.bytes_to_int32(ctypes.sizeof(xSendExtensionEventReq)) + stuff.count + lib.bytes_to_int32(stuff.num_events * ctypes.sizeof(xEvent))
    if stuff.length != expected_length:
        return lib.BadLength

    eventP = ctypes.cast(ctypes.byref(stuff, ctypes.sizeof(xSendExtensionEventReq)), POINTER(xEvent))
    for i in range(stuff.num_events):
        proc = lib.EventSwapVector[eventP[i].u.u.type & 0177]
        if proc == lib.NotImplemented:
            return lib.BadValue
        proc(byref(eventP[i]), byref(eventT))
        eventP[i] = eventT

    p = ctypes.cast(ctypes.byref(eventP[stuff.num_events]), POINTER(c_int))
    lib.SwapLongs(p, stuff.count)
    return ProcXSendExtensionEvent(client)