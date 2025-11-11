import ctypes
from ctypes.util import find_library

# Load Linux C standard library dynamically
libc = ctypes.CDLL(find_library('c'))

# C struct replacements using ctypes
class ctl_table_set(ctypes.Structure):
    _fields_ = []

class ctl_table_root(ctypes.Structure):
    _fields_ = [("lookup", ctypes.c_void_p), ("permissions", ctypes.c_void_p)]

class nsproxy(ctypes.Structure):
    _fields_ = [("net_ns", ctypes.c_void_p)]

class net(ctypes.Structure):
    _fields_ = [("sysctls", ctypes.c_void_p), ("user_ns", ctypes.c_void_p)]

# Replacing static functions with Python functions
def net_ctl_header_lookup(root, namespaces):
    return ctypes.cast(ctypes.byref(namespaces.contents.net_ns.contents.sysctls), ctypes.POINTER(ctl_table_set))

def is_seen(set):
    current = ctypes.c_void_p.in_dll(libc, 'current')
    return ctypes.byref(current.contents.nsproxy.contents.net_ns.contents.sysctls) == set

def net_ctl_permissions(head, table):
    net = ctypes.cast(head.contents.set, ctypes.POINTER(net))
    root_uid = make_kuid(net.contents.user_ns, 0)
    root_gid = make_kgid(net.contents.user_ns, 0)

    if ns_capable(net.contents.user_ns, CAP_NET_ADMIN) or uid_eq(root_uid, current_uid()):
        mode = (table.contents.mode >> 6) & 7
        return (mode << 6) | (mode << 3) | mode

    if gid_eq(root_gid, current_gid()):
        mode = (table.contents.mode >> 3) & 7
        return (mode << 3) | mode

    return table.contents.mode

net_sysctl_root = ctl_table_root()
net_sysctl_root.lookup = ctypes.cast(net_ctl_header_lookup, ctypes.c_void_p)
net_sysctl_root.permissions = ctypes.cast(net_ctl_permissions, ctypes.c_void_p)

def sysctl_net_init(net):
    setup_sysctl_set(ctypes.byref(net.contents.sysctls), ctypes.byref(net_sysctl_root), is_seen)
    return 0

def sysctl_net_exit(net):
    retire_sysctl_set(ctypes.byref(net.contents.sysctls))

class pernet_operations(ctypes.Structure):
    _fields_ = [("init", ctypes.c_void_p), ("exit", ctypes.c_void_p)]

sysctl_pernet_ops = pernet_operations()
sysctl_pernet_ops.init = ctypes.cast(sysctl_net_init, ctypes.c_void_p)
sysctl_pernet_ops.exit = ctypes.cast(sysctl_net_exit, ctypes.c_void_p)

net_header = ctypes.POINTER(ctl_table_header)()

def net_sysctl_init():
    empty = (ctypes.c_void_p * 1)()
    ret = -ENOMEM

    net_header = register_sysctl("net", empty)
    if not net_header:
        return ret

    ret = register_pernet_subsys(ctypes.byref(sysctl_pernet_ops))
    if ret:
        return ret

    register_sysctl_root(ctypes.byref(net_sysctl_root))
    return ret

def register_net_sysctl(net, path, table):
    return __register_sysctl_table(ctypes.byref(net.contents.sysctls), path, table)

def unregister_net_sysctl_table(header):
    unregister_sysctl_table(header)