import ctypes
from ctypes import c_int, c_void_p, POINTER, Structure, c_uint32
from ctypes.util import find_library

# Load C library
libc = ctypes.CDLL(find_library("c"), use_errno=True)

# Define C structures and functions using ctypes

class sockaddr(Structure):
    _fields_ = [("sa_family", c_uint32),
                ("sa_data", c_uint32 * 14)]

class sockaddr_in(Structure):
    _fields_ = [("sin_family", c_uint32),
                ("sin_port", c_uint32),
                ("sin_addr", c_uint32)]

class inet_sock(Structure):
    _fields_ = [("inet_saddr", c_uint32),
                ("inet_daddr", c_uint32),
                ("inet_sport", c_uint32),
                ("inet_dport", c_uint32),
                ("mc_index", c_uint32),
                ("mc_addr", c_uint32)]

class sock(Structure):
    _fields_ = [("sk_bound_dev_if", c_uint32),
                ("sk_protocol", c_uint32),
                ("sk_state", c_uint32),
                ("sk_prot", c_void_p)]

# Constants
AF_INET = 2
EAFNOSUPPORT = -97
EINVAL = -22
ENETUNREACH = -101
EACCES = -13
TCP_ESTABLISHED = 1

def ip4_datagram_connect(sk, uaddr, addr_len):
    inet = ctypes.cast(sk, POINTER(inet_sock)).contents
    usin = ctypes.cast(uaddr, POINTER(sockaddr_in)).contents
    err = c_int()

    if addr_len < ctypes.sizeof(sockaddr_in):
        return EINVAL

    if usin.sin_family != AF_INET:
        return EAFNOSUPPORT

    libc.lock_sock(sk)

    oif = sk.contents.sk_bound_dev_if
    saddr = inet.inet_saddr

    if libc.ipv4_is_multicast(usin.sin_addr):
        if not oif:
            oif = inet.mc_index
        if not saddr:
            saddr = inet.mc_addr

    rt = libc.ip_route_connect(None, usin.sin_addr, saddr, 0, oif, sk.contents.sk_protocol, inet.inet_sport, usin.sin_port, sk)
    
    if rt < 0:
        err.value = rt
        if err.value == ENETUNREACH:
            libc.IP_INC_STATS(None, None)
        libc.release_sock(sk)
        return err.value

    if (libc.sock_flag(sk, 0) & 0) and not libc.sock_flag(sk, 0):
        libc.ip_rt_put(rt)
        err.value = EACCES
        libc.release_sock(sk)
        return err.value

    if not inet.inet_saddr:
        inet.inet_saddr = libc.fl4_saddr(None)
    if not inet.inet_rcv_saddr:
        inet.inet_rcv_saddr = libc.fl4_saddr(None)
        if libc.sk_prot_rehash(sk):
            libc.sk_prot_rehash(sk)

    inet.inet_daddr = libc.fl4_daddr(None)
    inet.inet_dport = usin.sin_port
    sk.contents.sk_state = TCP_ESTABLISHED

    libc.sk_dst_set(sk, None)
    err.value = 0
    libc.release_sock(sk)
    return err.value

libc.EXPORT_SYMBOL(ip4_datagram_connect)

def ip4_datagram_release_cb(sk):
    inet = ctypes.cast(sk, POINTER(inet_sock)).contents
    daddr = inet.inet_daddr
    fl4 = sockaddr_in()
    rt = libc.ip_route_output_ports(None, ctypes.byref(fl4), sk, daddr, inet.inet_saddr, inet.inet_dport, inet.inet_sport, sk.contents.sk_protocol, 0, sk.contents.sk_bound_dev_if)

    if rt >= 0:
        libc.__sk_dst_set(sk, None)

libc.EXPORT_SYMBOL_GPL(ip4_datagram_release_cb)