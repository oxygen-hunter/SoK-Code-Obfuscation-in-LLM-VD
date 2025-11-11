import ctypes
import os

# Load C library
libc_path = "libc.so.6" if os.name != "nt" else "msvcrt.dll"
libc = ctypes.CDLL(libc_path)

# Define C structures and functions using ctypes
class RAnalBlock(ctypes.Structure):
    _fields_ = [
        ("addr", ctypes.c_uint64),
        ("jump", ctypes.c_uint64),
        ("fail", ctypes.c_uint64),
        ("switch_op", ctypes.c_void_p),
        ("type", ctypes.c_int),
        ("cond", ctypes.c_void_p),
        ("fingerprint", ctypes.c_void_p),
        ("diff", ctypes.c_void_p),
        ("label", ctypes.c_void_p),
        ("op_pos", ctypes.POINTER(ctypes.c_uint16)),
        ("op_pos_size", ctypes.c_int),
        ("parent_reg_arena", ctypes.c_void_p),
        ("stackptr", ctypes.c_int),
        ("parent_stackptr", ctypes.c_int),
        ("prev", ctypes.c_void_p),
        ("jumpbb", ctypes.c_void_p),
        ("failbb", ctypes.c_void_p),
        ("size", ctypes.c_int),
        ("ninstr", ctypes.c_int),
        ("conditional", ctypes.c_int),
    ]

class RAnal(ctypes.Structure):
    _fields_ = []

R_API = ctypes.CFUNCTYPE

def r_anal_bb_new():
    bb = libc.malloc(ctypes.sizeof(RAnalBlock))
    if not bb:
        return None
    bb = ctypes.cast(bb, ctypes.POINTER(RAnalBlock))
    bb.contents.addr = ctypes.c_uint64(-1)
    bb.contents.jump = ctypes.c_uint64(-1)
    bb.contents.fail = ctypes.c_uint64(-1)
    bb.contents.switch_op = None
    bb.contents.type = 0
    bb.contents.cond = None
    bb.contents.fingerprint = None
    bb.contents.diff = None
    bb.contents.label = None
    bb.contents.op_pos = libc.calloc(3, ctypes.sizeof(ctypes.c_uint16))
    bb.contents.op_pos_size = 3
    bb.contents.parent_reg_arena = None
    bb.contents.stackptr = 0
    bb.contents.parent_stackptr = ctypes.c_int(0x7FFFFFFF)
    return bb

def r_anal_bb_free(bb):
    if not bb:
        return
    libc.free(bb.contents.fingerprint)
    libc.free(bb.contents.diff)
    libc.free(bb.contents.op_pos)
    libc.free(bb.contents.label)
    libc.free(bb.contents.parent_reg_arena)
    libc.free(ctypes.byref(bb))

def r_anal_bb_list_new():
    list_ptr = libc.malloc(ctypes.sizeof(ctypes.c_void_p))
    if not list_ptr:
        return None
    return list_ptr

def r_anal_bb(anal, bb, addr, buf, length, head):
    if bb.contents.addr == -1:
        bb.contents.addr = addr
    length -= 16
    idx = 0
    while idx < length:
        op = libc.malloc(ctypes.sizeof(RAnalBlock))
        if not op:
            return -1
        oplen = libc.memcmp(buf + idx, b'\x90', 1)
        if oplen == 0:
            libc.free(op)
            if idx == 0:
                return -2
            break
        if oplen < 1:
            libc.free(op)
            return -2
        idx += oplen
        bb.contents.size += oplen
        libc.free(op)
    return bb.contents.size

def r_anal_bb_is_in_offset(bb, off):
    return off >= bb.contents.addr and off < bb.contents.addr + bb.contents.size

def r_anal_bb_from_offset(anal, off):
    return None

def r_anal_bb_get_jumpbb(fcn, bb):
    return None

def r_anal_bb_get_failbb(fcn, bb):
    return None

def r_anal_bb_offset_inst(bb, i):
    if i < 0 or i >= bb.contents.ninstr:
        return ctypes.c_uint16(-1)
    return bb.contents.op_pos[i - 1] if i > 0 and (i - 1) < bb.contents.op_pos_size else 0

def r_anal_bb_set_offset(bb, i, v):
    if i > 0 and v > 0:
        if i >= bb.contents.op_pos_size:
            new_pos_size = i * 2
            tmp_op_pos = libc.realloc(bb.contents.op_pos, new_pos_size * ctypes.sizeof(ctypes.c_uint16))
            if not tmp_op_pos:
                return False
            bb.contents.op_pos_size = new_pos_size
            bb.contents.op_pos = tmp_op_pos
        bb.contents.op_pos[i - 1] = v
        return True
    return True

def r_anal_bb_opaddr_at(bb, off):
    if not r_anal_bb_is_in_offset(bb, off):
        return ctypes.c_uint64(-1)
    last_delta = 0
    delta_off = off - bb.contents.addr
    for i in range(bb.contents.ninstr):
        delta = r_anal_bb_offset_inst(bb, i)
        if delta > delta_off:
            return bb.contents.addr + last_delta
        last_delta = delta
    return ctypes.c_uint64(-1)