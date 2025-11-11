# Import Python's ctypes to interact with C libraries
from ctypes import CDLL, c_int, POINTER, Structure, c_char_p, byref
import os

# Load the C standard library
libc = CDLL("libc.so.6")

class lxc_lock(Structure):
    pass

class lxc_container(Structure):
    _fields_ = [("privlock", POINTER(lxc_lock)),
                ("slock", POINTER(lxc_lock))]

libc.sem_init.restype = c_int
libc.sem_init.argtypes = [POINTER(c_int), c_int, c_int]
libc.sem_wait.restype = c_int
libc.sem_wait.argtypes = [POINTER(c_int)]
libc.sem_post.restype = c_int
libc.sem_post.argtypes = [POINTER(c_int)]
libc.sem_destroy.restype = c_int
libc.sem_destroy.argtypes = [POINTER(c_int)]

def lxc_new_unnamed_sem():
    s = (c_int * 1)()
    if libc.sem_init(s, 0, 1) != 0:
        return None
    return s

def lxclock(l, timeout):
    if l.contents.type == "LXC_LOCK_ANON_SEM":
        if libc.sem_wait(l.contents.u.sem) == -1:
            return -1
    return 0

def lxcunlock(l):
    if l.contents.type == "LXC_LOCK_ANON_SEM":
        if libc.sem_post(l.contents.u.sem) == -1:
            return -1
    return 0

def lxc_putlock(l):
    if l.contents.type == "LXC_LOCK_ANON_SEM":
        if l.contents.u.sem:
            libc.sem_destroy(l.contents.u.sem)
            l.contents.u.sem = None

def container_mem_lock(c):
    return lxclock(c.contents.privlock, 0)

def container_mem_unlock(c):
    lxcunlock(c.contents.privlock)

def container_disk_lock(c):
    if lxclock(c.contents.privlock, 0) != 0:
        return -1
    if lxclock(c.contents.slock, 0) != 0:
        lxcunlock(c.contents.privlock)
        return -1
    return 0

def container_disk_unlock(c):
    lxcunlock(c.contents.slock)
    lxcunlock(c.contents.privlock)