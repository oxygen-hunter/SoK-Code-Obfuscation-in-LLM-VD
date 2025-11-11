# SPDX-License-Identifier: MIT
#
# tftp.py - a simple, read-only tftp server for qemu
#
# Copyright (c) 2004 Magnus Damm <damm@opensource.se>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import ctypes
import os

# Load C functions from a shared library
libc = ctypes.CDLL(None)

class TftpSession(ctypes.Structure):
    _fields_ = [
        ("slirp", ctypes.c_void_p),
        ("timestamp", ctypes.c_int),
        ("fd", ctypes.c_int),
        ("filename", ctypes.c_char_p),
        ("client_addr", ctypes.c_void_p),
        ("client_port", ctypes.c_int),
        ("block_size", ctypes.c_int),
        ("block_nr", ctypes.c_uint32)
    ]

def tftp_session_in_use(spt):
    return bool(spt.slirp)

def tftp_session_update(spt):
    spt.timestamp = libc.time(None)

def tftp_session_terminate(spt):
    if spt.fd >= 0:
        os.close(spt.fd)
        spt.fd = -1
    
    if spt.filename:
        libc.free(spt.filename)
    
    spt.slirp = None

def tftp_session_allocate(slirp, srcsas, tp):
    TFTP_SESSIONS_MAX = 5
    sessions = (TftpSession * TFTP_SESSIONS_MAX).in_dll(libc, "tftp_sessions")
    curtime = libc.time(None)
    
    for k in range(TFTP_SESSIONS_MAX):
        spt = sessions[k]

        if not tftp_session_in_use(spt):
            break

        if (curtime - spt.timestamp) > 5000:
            tftp_session_terminate(spt)
            break
    else:
        return -1
    
    libc.memset(ctypes.byref(spt), 0, ctypes.sizeof(spt))
    libc.memcpy(ctypes.byref(spt.client_addr), srcsas, libc.sockaddr_size(srcsas))
    spt.fd = -1
    spt.block_size = 512
    spt.client_port = tp.udp.uh_sport
    spt.slirp = slirp

    tftp_session_update(spt)

    return k

def tftp_input(srcsas, m):
    tp = ctypes.cast(m.m_data, ctypes.POINTER(TftpSession)).contents
    op = libc.ntohs(tp.tp_op)

    if op == 1:  # TFTP_RRQ
        tftp_handle_rrq(m.slirp, srcsas, tp, m.m_len)
    elif op == 4:  # TFTP_ACK
        tftp_handle_ack(m.slirp, srcsas, tp, m.m_len)
    elif op == 5:  # TFTP_ERROR
        tftp_handle_error(m.slirp, srcsas, tp, m.m_len)

def tftp_handle_rrq(slirp, srcsas, tp, pktlen):
    # Handle read request
    pass

def tftp_handle_ack(slirp, srcsas, tp, pktlen):
    # Handle acknowledgment
    pass

def tftp_handle_error(slirp, srcsas, tp, pktlen):
    # Handle error
    pass