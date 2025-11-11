import ctypes
from ctypes import c_int, c_long, POINTER, Structure

class TaskStruct(Structure):
    _fields_ = [("ptrace", c_long)]

class PtRegs(Structure):
    _fields_ = [
        ("pc", c_long), ("ps", c_long), ("lbeg", c_long),
        ("lend", c_long), ("lcount", c_long), ("windowstart", c_long),
        ("windowbase", c_long), ("areg", c_long * 16), ("sar", c_long),
        ("syscall", c_long), ("xtregs_opt", c_long), ("xtregs_user", c_long)
    ]

class ElfXtregs_t(Structure):
    _fields_ = [("cp0", c_long), ("opt", c_long), ("user", c_long)]

class XtensaGregset_t(Structure):
    _fields_ = [("pc", c_long), ("ps", c_long), ("lbeg", c_long),
                ("lend", c_long), ("lcount", c_long), ("windowstart", c_long),
                ("windowbase", c_long), ("a", c_long * 16)]

PT_SINGLESTEP = 0x100
EIO = -5
EFAULT = -14
EPERM = -1
WSBITS = 6
PS_CALLINC_MASK = 0x3
PS_OWB_MASK = 0x1
PS_EXCM_BIT = 0x20
XCHAL_NUM_AREGS = 16
PTRACE_GETREGS = 12
PTRACE_SETREGS = 13
PTRACE_GETXTREGS = 18
PTRACE_SETXTREGS = 19
PTRACE_PEEKTEXT = 1
PTRACE_PEEKDATA = 2
PTRACE_PEEKUSR = 3
PTRACE_POKETEXT = 4
PTRACE_POKEDATA = 5
PTRACE_POKEUSR = 6

libc = ctypes.CDLL(None)
libc.access_ok.argtypes = [c_long, c_long, c_long]
libc.access_ok.restype = c_int

def user_enable_single_step(child_ptr):
    child = ctypes.cast(child_ptr, POINTER(TaskStruct))
    child.contents.ptrace |= PT_SINGLESTEP
    
def user_disable_single_step(child_ptr):
    child = ctypes.cast(child_ptr, POINTER(TaskStruct))
    child.contents.ptrace &= ~PT_SINGLESTEP

def ptrace_disable(child_ptr):
    pass

def ptrace_getregs(child_ptr, uregs):
    child = ctypes.cast(child_ptr, POINTER(TaskStruct))
    regs = PtRegs.in_dll(libc, "task_pt_regs")(child)
    gregset = ctypes.cast(uregs, POINTER(XtensaGregset_t))
    wm = regs.contents.wmask
    wb = regs.contents.windowbase
    live = (wm & 2) and 4 or (wm & 4) and 8 or (wm & 8) and 12 or 16

    if not libc.access_ok(1, uregs, ctypes.sizeof(XtensaGregset_t)):
        return EIO

    for i in range(live):
        OFFSET = ((wb * 4 + i) % XCHAL_NUM_AREGS)
        gregset.contents.a[OFFSET] = regs.contents.areg[i]

    for i in range(XCHAL_NUM_AREGS - (wm >> 4) * 4, XCHAL_NUM_AREGS):
        OFFSET = ((wb * 4 + i) % XCHAL_NUM_AREGS)
        gregset.contents.a[OFFSET] = regs.contents.areg[i]

    return 0

def ptrace_setregs(child_ptr, uregs):
    child = ctypes.cast(child_ptr, POINTER(TaskStruct))
    regs = PtRegs.in_dll(libc, "task_pt_regs")(child)
    gregset = ctypes.cast(uregs, POINTER(XtensaGregset_t))
    ps_mask = PS_CALLINC_MASK | PS_OWB_MASK
    ps = gregset.contents.ps
    wb = gregset.contents.windowbase

    if not libc.access_ok(1, uregs, ctypes.sizeof(XtensaGregset_t)):
        return EIO

    regs.contents.ps = ((regs.contents.ps & ~ps_mask) | (ps & ps_mask) | (1 << PS_EXCM_BIT))

    if wb >= XCHAL_NUM_AREGS / 4:
        return EFAULT

    regs.contents.windowbase = wb

    return 0

def arch_ptrace(child_ptr, request, addr, data):
    child = ctypes.cast(child_ptr, POINTER(TaskStruct))
    ret = EPERM
    datap = ctypes.cast(data, c_long)

    if request in (PTRACE_PEEKTEXT, PTRACE_PEEKDATA):
        ret = generic_ptrace_peekdata(child, addr, data)
    elif request == PTRACE_PEEKUSR:
        ret = ptrace_peekusr(child, addr, datap)
    elif request in (PTRACE_POKETEXT, PTRACE_POKEDATA):
        ret = generic_ptrace_pokedata(child, addr, data)
    elif request == PTRACE_POKEUSR:
        ret = ptrace_pokeusr(child, addr, data)
    elif request == PTRACE_GETREGS:
        ret = ptrace_getregs(child, datap)
    elif request == PTRACE_SETREGS:
        ret = ptrace_setregs(child, datap)
    elif request == PTRACE_GETXTREGS:
        ret = ptrace_getxregs(child, datap)
    elif request == PTRACE_SETXTREGS:
        ret = ptrace_setxregs(child, datap)
    else:
        ret = ptrace_request(child, request, addr, data)

    return ret