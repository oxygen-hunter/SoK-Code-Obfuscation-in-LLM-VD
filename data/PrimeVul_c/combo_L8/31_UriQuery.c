import ctypes
import os

# Load the shared library
lib = ctypes.CDLL(os.path.join(os.getcwd(), 'liburiparser.so'))

# Define return types and argument types for the C functions
lib.ComposeQueryCharsRequired.restype = ctypes.c_int
lib.ComposeQueryCharsRequired.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]

lib.ComposeQuery.restype = ctypes.c_int
lib.ComposeQuery.argtypes = [ctypes.c_char_p, ctypes.c_void_p, ctypes.c_int, ctypes.POINTER(ctypes.c_int)]

lib.ComposeQueryMalloc.restype = ctypes.c_int
lib.ComposeQueryMalloc.argtypes = [ctypes.POINTER(ctypes.c_char_p), ctypes.c_void_p]

lib.DissectQueryMalloc.restype = ctypes.c_int
lib.DissectQueryMalloc.argtypes = [ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_int), ctypes.c_char_p, ctypes.c_char_p]

def ComposeQueryCharsRequired(queryList):
    charsRequired = ctypes.c_int()
    result = lib.ComposeQueryCharsRequired(queryList, ctypes.byref(charsRequired))
    if result != 0:
        raise Exception("Error in ComposeQueryCharsRequired")
    return charsRequired.value

def ComposeQuery(dest, queryList, maxChars):
    charsWritten = ctypes.c_int()
    result = lib.ComposeQuery(dest, queryList, maxChars, ctypes.byref(charsWritten))
    if result != 0:
        raise Exception("Error in ComposeQuery")
    return charsWritten.value

def ComposeQueryMalloc(queryList):
    dest = ctypes.c_char_p()
    result = lib.ComposeQueryMalloc(ctypes.byref(dest), queryList)
    if result != 0:
        raise Exception("Error in ComposeQueryMalloc")
    return dest.value

def DissectQueryMalloc(first, afterLast):
    dest = ctypes.c_void_p()
    itemCount = ctypes.c_int()
    result = lib.DissectQueryMalloc(ctypes.byref(dest), ctypes.byref(itemCount), first.encode('utf-8'), afterLast.encode('utf-8'))
    if result != 0:
        raise Exception("Error in DissectQueryMalloc")
    return dest, itemCount.value