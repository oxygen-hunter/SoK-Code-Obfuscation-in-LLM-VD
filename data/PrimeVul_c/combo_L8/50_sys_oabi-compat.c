# Python wrapper for C functions

import ctypes
import os

# Load the C standard library
libc = ctypes.CDLL(None)

class oldabi_stat64(ctypes.Structure):
    _fields_ = [
        ("st_dev", ctypes.c_uint64),
        ("__pad1", ctypes.c_uint32),
        ("__st_ino", ctypes.c_ulong),
        ("st_mode", ctypes.c_uint32),
        ("st_nlink", ctypes.c_uint32),
        ("st_uid", ctypes.c_ulong),
        ("st_gid", ctypes.c_ulong),
        ("st_rdev", ctypes.c_uint64),
        ("__pad2", ctypes.c_uint32),
        ("st_size", ctypes.c_longlong),
        ("st_blksize", ctypes.c_ulong),
        ("st_blocks", ctypes.c_ulonglong),
        ("st_atime", ctypes.c_ulong),
        ("st_atime_nsec", ctypes.c_ulong),
        ("st_mtime", ctypes.c_ulong),
        ("st_mtime_nsec", ctypes.c_ulong),
        ("st_ctime", ctypes.c_ulong),
        ("st_ctime_nsec", ctypes.c_ulong),
        ("st_ino", ctypes.c_ulonglong),
    ]

def sys_oabi_stat64(filename):
    statbuf = oldabi_stat64()
    filename_c = ctypes.create_string_buffer(bytes(filename, 'utf-8'))
    result = libc.stat(filename_c, ctypes.byref(statbuf))
    if result == 0:
        return statbuf
    else:
        raise OSError(ctypes.get_errno())

def sys_oabi_lstat64(filename):
    statbuf = oldabi_stat64()
    filename_c = ctypes.create_string_buffer(bytes(filename, 'utf-8'))
    result = libc.lstat(filename_c, ctypes.byref(statbuf))
    if result == 0:
        return statbuf
    else:
        raise OSError(ctypes.get_errno())

def sys_oabi_fstat64(fd):
    statbuf = oldabi_stat64()
    result = libc.fstat(fd, ctypes.byref(statbuf))
    if result == 0:
        return statbuf
    else:
        raise OSError(ctypes.get_errno())

def sys_oabi_fstatat64(dfd, filename, flag):
    statbuf = oldabi_stat64()
    filename_c = ctypes.create_string_buffer(bytes(filename, 'utf-8'))
    result = libc.fstatat(dfd, filename_c, ctypes.byref(statbuf), flag)
    if result == 0:
        return statbuf
    else:
        raise OSError(ctypes.get_errno())

# Example usage
try:
    stat64 = sys_oabi_stat64("/path/to/file")
    print("Size:", stat64.st_size)
except OSError as e:
    print("Error:", e)