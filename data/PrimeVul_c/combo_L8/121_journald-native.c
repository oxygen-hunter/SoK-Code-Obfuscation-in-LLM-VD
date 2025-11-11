# SPDX-License-Identifier: LGPL-2.1+

import ctypes
import ctypes.util
import mmap
import os
import socket
import sys

libc = ctypes.CDLL(ctypes.util.find_library('c'))

class ucred(ctypes.Structure):
    _fields_ = [("pid", ctypes.c_int),
                ("uid", ctypes.c_uint),
                ("gid", ctypes.c_uint)]

def allow_object_pid(ucred):
    return ucred and ucred.uid == 0

def server_process_entry_meta(p, l, ucred, priority, identifier, message, object_pid):
    if l == 10 and p.startswith("PRIORITY=") and '0' <= p[9] <= '9':
        priority[0] = (priority[0] & libc.LOG_FACMASK) | (ord(p[9]) - ord('0'))
    elif l == 17 and p.startswith("SYSLOG_FACILITY=") and '0' <= p[16] <= '9':
        priority[0] = (priority[0] & libc.LOG_PRIMASK) | ((ord(p[16]) - ord('0')) << 3)
    elif l == 18 and p.startswith("SYSLOG_FACILITY=") and '0' <= p[16] <= '9' and '0' <= p[17] <= '9':
        priority[0] = (priority[0] & libc.LOG_PRIMASK) | (((ord(p[16]) - ord('0'))*10 + (ord(p[17]) - ord('0'))) << 3)
    elif l >= 19 and p.startswith("SYSLOG_IDENTIFIER="):
        identifier[0] = p[18:l]
    elif l >= 8 and p.startswith("MESSAGE="):
        message[0] = p[8:l]
    elif l > len("OBJECT_PID=") and l < len("OBJECT_PID=") + libc.DECIMAL_STR_MAX(ctypes.c_int) and p.startswith("OBJECT_PID=") and allow_object_pid(ucred):
        buf = p[len("OBJECT_PID="):l]
        object_pid[0] = int(buf)

def server_process_native_message(s, buffer, buffer_size, ucred, tv, label, label_len):
    remaining = buffer_size
    context = None
    r = 0

    if ucred and libc.pid_is_valid(ucred.pid):
        r, context = libc.client_context_get(s, ucred.pid, ucred, label, label_len, None, ctypes.pointer(context))
        if r < 0:
            libc.log_warning_errno(r, f"Failed to retrieve credentials for PID {ucred.pid}, ignoring: %m")

    while r == 0:
        r = server_process_entry(s, buffer[buffer_size - remaining:], remaining, context, ucred, tv, label, label_len)

def server_process_native_file(s, fd, ucred, tv, label, label_len):
    st = os.fstat(fd)
    sealed = libc.memfd_get_sealed(fd) > 0

    if not sealed and (not ucred or ucred.uid != 0):
        k, _ = libc.fd_get_path(fd)
        if not k:
            return

        e = k if any(k.startswith(prefix) for prefix in ["/dev/shm/", "/tmp/", "/var/tmp/"]) else None
        if not e or not libc.filename_is_valid(e):
            return

    if not os.S_ISREG(st.st_mode) or st.st_size <= 0 or st.st_size > libc.ENTRY_SIZE_MAX:
        return

    if sealed:
        ps = libc.PAGE_ALIGN(st.st_size)
        p = mmap.mmap(fd, ps, mmap.PROT_READ, mmap.MAP_PRIVATE)
        server_process_native_message(s, p, st.st_size, ucred, tv, label, label_len)
        p.close()
    else:
        vfs = os.statvfs(fd)
        if vfs.f_flag & libc.ST_MANDLOCK:
            return

        libc.fd_nonblock(fd, True)

        p = os.read(fd, st.st_size)
        if p:
            server_process_native_message(s, p, len(p), ucred, tv, label, label_len)

class Server(ctypes.Structure):
    _fields_ = [("native_fd", ctypes.c_int),
                ("event", ctypes.c_void_p),
                ("native_event_source", ctypes.c_void_p)]

def server_open_native_socket(s):
    sa = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    s.native_fd = sa.fileno()

    sa.bind("/run/systemd/journal/socket")
    os.chmod("/run/systemd/journal/socket", 0o666)

    libc.setsockopt_int(s.native_fd, socket.SOL_SOCKET, socket.SO_PASSCRED, 1)
    if libc.mac_selinux_use():
        libc.setsockopt_int(s.native_fd, socket.SOL_SOCKET, socket.SO_PASSSEC, 1)

    libc.setsockopt_int(s.native_fd, socket.SOL_SOCKET, socket.SO_TIMESTAMP, 1)
    libc.sd_event_add_io(s.event, s.native_event_source, s.native_fd, libc.EPOLLIN, libc.server_process_datagram, s)
    libc.sd_event_source_set_priority(s.native_event_source, libc.SD_EVENT_PRIORITY_NORMAL + 5)