# Python wrapper for C code using ctypes
import ctypes
import os
import threading
from ctypes import c_int, c_void_p, POINTER, Structure


# Load the shared library
lib = ctypes.CDLL('./liblxcapi.so')

# Define C data structures in Python
class LxcContainer(Structure):
    _fields_ = [("privlock", c_void_p), ("slock", c_void_p)]

class LxcLock(Structure):
    _fields_ = [("type", c_int), ("u", c_void_p)]

# Python lock for thread safety
thread_mutex = threading.Lock()

def lock_mutex():
    with thread_mutex:
        pass

def unlock_mutex():
    with thread_mutex:
        pass

def process_lock():
    lock_mutex()

def process_unlock():
    unlock_mutex()

# Function bindings from the C library
lib.container_mem_lock.argtypes = [POINTER(LxcContainer)]
lib.container_mem_lock.restype = c_int

lib.container_mem_unlock.argtypes = [POINTER(LxcContainer)]
lib.container_mem_unlock.restype = None

lib.container_disk_lock.argtypes = [POINTER(LxcContainer)]
lib.container_disk_lock.restype = c_int

lib.container_disk_unlock.argtypes = [POINTER(LxcContainer)]
lib.container_disk_unlock.restype = None

def container_mem_lock(c):
    return lib.container_mem_lock(ctypes.byref(c))

def container_mem_unlock(c):
    lib.container_mem_unlock(ctypes.byref(c))

def container_disk_lock(c):
    ret = lib.container_disk_lock(ctypes.byref(c))
    if ret:
        return ret
    return 0

def container_disk_unlock(c):
    lib.container_disk_unlock(ctypes.byref(c))
    lib.container_mem_unlock(ctypes.byref(c))