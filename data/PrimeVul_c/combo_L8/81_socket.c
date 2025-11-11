# socket.py

from ctypes import CDLL, c_int, c_char_p, c_void_p, POINTER, c_size_t, c_uint16, c_uint, c_uint32
import os
import socket
import sys

libc = CDLL("libc.so.6")
RECV_TIMEOUT = 20000
verbose = 0

def socket_set_verbose(level: int):
    global verbose
    verbose = level

def socket_create_unix(filename: str) -> int:
    name = sockaddr_un()
    sock = libc.socket(socket.AF_UNIX, socket.SOCK_STREAM, 0)
    if sock < 0:
        libc.perror(b"socket")
        return -1

    os.unlink(filename)
    name.sun_family = socket.AF_UNIX
    name.sun_path = filename.encode('utf-8')
    size = len(name.sun_path) + libc.offsetof(sockaddr_un, b"sun_path") + 1

    if libc.bind(sock, c_void_p(name), size) < 0:
        libc.perror(b"bind")
        socket_close(sock)
        return -1

    if libc.listen(sock, 10) < 0:
        libc.perror(b"listen")
        socket_close(sock)
        return -1

    return sock

def socket_connect_unix(filename: str) -> int:
    name = sockaddr_un()
    sfd = libc.socket(socket.AF_UNIX, socket.SOCK_STREAM, 0)
    if sfd < 0:
        libc.perror(b"socket")
        return -1

    name.sun_family = socket.AF_UNIX
    name.sun_path = filename.encode('utf-8')
    size = len(name.sun_path) + libc.offsetof(sockaddr_un, b"sun_path") + 1

    if libc.connect(sfd, c_void_p(name), size) < 0:
        socket_close(sfd)
        libc.perror(b"connect")
        return -1

    return sfd

def socket_create(port: int) -> int:
    sfd = libc.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
    if sfd < 0:
        libc.perror(b"socket()")
        return -1

    yes = c_int(1)
    if libc.setsockopt(sfd, socket.SOL_SOCKET, socket.SO_REUSEADDR, c_void_p(yes), c_size_t(4)) == -1:
        libc.perror(b"setsockopt()")
        socket_close(sfd)
        return -1

    saddr = sockaddr_in()
    saddr.sin_family = socket.AF_INET
    saddr.sin_addr.s_addr = socket.htonl(socket.INADDR_ANY)
    saddr.sin_port = socket.htons(port)

    if libc.bind(sfd, c_void_p(saddr), c_size_t(len(saddr))) < 0:
        libc.perror(b"bind()")
        socket_close(sfd)
        return -1

    if libc.listen(sfd, 1) == -1:
        libc.perror(b"listen()")
        socket_close(sfd)
        return -1

    return sfd

def socket_connect(addr: str, port: int) -> int:
    sfd = libc.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
    if sfd < 0:
        libc.perror(b"socket()")
        return -1

    hp = socket.gethostbyname(addr)
    if not hp:
        libc.perror(b"gethostbyname()")
        return -1

    yes = c_int(1)
    if libc.setsockopt(sfd, socket.SOL_SOCKET, socket.SO_REUSEADDR, c_void_p(yes), c_size_t(4)) == -1:
        libc.perror(b"setsockopt()")
        socket_close(sfd)
        return -1

    saddr = sockaddr_in()
    saddr.sin_family = socket.AF_INET
    saddr.sin_addr.s_addr = c_uint32(int.from_bytes(socket.inet_aton(hp), 'big'))
    saddr.sin_port = socket.htons(port)

    if libc.connect(sfd, c_void_p(saddr), c_size_t(len(saddr))) < 0:
        libc.perror(b"connect()")
        socket_close(sfd)
        return -1

    return sfd

def socket_check_fd(fd: int, fdm: int, timeout: int) -> int:
    if fd <= 0:
        libc.perror(b"invalid fd")
        return -1

    fds = fd_set()
    libc.FD_ZERO(c_void_p(fds))
    libc.FD_SET(fd, c_void_p(fds))

    to = timeval()
    if timeout > 0:
        to.tv_sec = timeout // 1000
        to.tv_usec = (timeout % 1000) * 1000
        pto = c_void_p(to)
    else:
        pto = None

    sret = libc.select(fd + 1, c_void_p(fds), None, None, pto)
    return sret

def socket_accept(fd: int, port: int) -> int:
    addr = sockaddr_in()
    addr.sin_family = socket.AF_INET
    addr.sin_addr.s_addr = socket.htonl(socket.INADDR_ANY)
    addr.sin_port = socket.htons(port)

    addr_len = c_size_t(len(addr))
    result = libc.accept(fd, c_void_p(addr), c_void_p(addr_len))
    return result

def socket_shutdown(fd: int, how: int) -> int:
    return libc.shutdown(fd, how)

def socket_close(fd: int) -> int:
    return libc.close(fd)

def socket_receive(fd: int, data: c_void_p, length: int) -> int:
    return socket_receive_timeout(fd, data, length, 0, RECV_TIMEOUT)

def socket_peek(fd: int, data: c_void_p, length: int) -> int:
    return socket_receive_timeout(fd, data, length, 2, RECV_TIMEOUT)

def socket_receive_timeout(fd: int, data: c_void_p, length: int, flags: int, timeout: int) -> int:
    res = socket_check_fd(fd, 0, timeout)
    if res <= 0:
        return res
    result = libc.recv(fd, data, length, flags)
    if res > 0 and result == 0:
        libc.perror(b"recv() returned 0")
        return -11  # EAGAIN
    if result < 0:
        return -errno
    return result

def socket_send(fd: int, data: c_void_p, length: int) -> int:
    return libc.send(fd, data, length, 0)

class sockaddr_un:
    def __init__(self):
        self.sun_family = c_int()
        self.sun_path = b'\x00' * 108

class sockaddr_in:
    def __init__(self):
        self.sin_family = c_int()
        self.sin_port = c_uint16()
        self.sin_addr = in_addr()

class in_addr:
    def __init__(self):
        self.s_addr = c_uint32()

class timeval:
    def __init__(self):
        self.tv_sec = c_int()
        self.tv_usec = c_int()

class fd_set:
    def __init__(self):
        self.fd_count = c_int()
        self.fd_array = (c_int * 1024)()