import ctypes, os
from ctypes import c_int64, c_double, c_int, Structure, POINTER, byref, cdll

# Load the C library
lib = cdll.LoadLibrary(os.path.join(os.path.dirname(__file__), "fps_filter.so"))

class AVRational(Structure):
    _fields_ = [("num", c_int), ("den", c_int)]

class FPSContext(Structure):
    _fields_ = [
        ("class", ctypes.c_void_p),
        ("fifo", ctypes.c_void_p),
        ("first_pts", c_int64),
        ("pts", c_int64),
        ("start_time", c_double),
        ("framerate", AVRational),
        ("rounding", c_int),
        ("frames_in", c_int),
        ("frames_out", c_int),
        ("dup", c_int),
        ("drop", c_int)
    ]

class AVFilterContext(Structure):
    _fields_ = [
        ("priv", POINTER(FPSContext)),
        ("inputs", ctypes.c_void_p),
        ("outputs", ctypes.c_void_p)
    ]

class AVFrame(Structure):
    _fields_ = [
        ("pts", c_int64)
    ]

def init(ctx):
    s = ctx.contents.priv.contents
    if not lib.av_fifo_alloc(2 * ctypes.sizeof(ctypes.c_void_p)):
        return -12  # AVERROR(ENOMEM)
    s.pts = -9223372036854775808  # AV_NOPTS_VALUE
    s.first_pts = -9223372036854775808  # AV_NOPTS_VALUE
    print(f"fps={s.framerate.num}/{s.framerate.den}")
    return 0

def flush_fifo(fifo):
    while lib.av_fifo_size(fifo):
        tmp = ctypes.c_void_p()
        lib.av_fifo_generic_read(fifo, byref(tmp), ctypes.sizeof(tmp), None)
        lib.av_frame_free(ctypes.byref(tmp))

def uninit(ctx):
    s = ctx.contents.priv.contents
    if s.fifo:
        s.drop += lib.av_fifo_size(s.fifo) // ctypes.sizeof(ctypes.c_void_p)
        flush_fifo(s.fifo)
        lib.av_fifo_free(s.fifo)
    print(f"{s.frames_in} frames in, {s.frames_out} frames out; {s.drop} frames dropped, {s.dup} frames duplicated.")

def config_props(link):
    s = link.contents.src.contents.priv.contents
    link.contents.time_base = lib.av_inv_q(s.framerate)
    link.contents.frame_rate = s.framerate
    link.contents.w = link.contents.src.contents.inputs[0].contents.w
    link.contents.h = link.contents.src.contents.inputs[0].contents.h
    return 0

def request_frame(outlink):
    ctx = outlink.contents.src
    s = ctx.contents.priv.contents
    frames_out = s.frames_out
    ret = 0
    while ret >= 0 and s.frames_out == frames_out:
        ret = lib.ff_request_frame(ctx.contents.inputs[0])
    if ret == -541478725 and lib.av_fifo_size(s.fifo):  # AVERROR_EOF
        for _ in range(lib.av_fifo_size(s.fifo)):
            buf = ctypes.c_void_p()
            lib.av_fifo_generic_read(s.fifo, byref(buf), ctypes.sizeof(buf), None)
            buf.contents.pts = lib.av_rescale_q(s.first_pts, ctx.contents.inputs[0].contents.time_base, outlink.contents.time_base) + s.frames_out
            if (ret := lib.ff_filter_frame(outlink.contents, buf)) < 0:
                return ret
            s.frames_out += 1
        return 0
    return ret

def write_to_fifo(fifo, buf):
    if not lib.av_fifo_space(fifo) and (ret := lib.av_fifo_realloc2(fifo, 2 * lib.av_fifo_size(fifo))):
        lib.av_frame_free(ctypes.byref(buf))
        return ret
    lib.av_fifo_generic_write(fifo, byref(buf), ctypes.sizeof(buf), None)
    return 0

def filter_frame(inlink, buf):
    ctx = inlink.contents.dst
    s = ctx.contents.priv.contents
    outlink = ctx.contents.outputs[0]
    s.frames_in += 1
    if s.pts == -9223372036854775808:
        if buf.contents.pts != -9223372036854775808:
            if (ret := write_to_fifo(s.fifo, buf)) < 0:
                return ret
            if s.start_time != 1.7976931348623157e+308 and s.start_time != -9223372036854775808:
                first_pts = s.start_time * 1000000.0
                first_pts = min(max(first_pts, -9223372036854775808), 9223372036854775807)
                s.first_pts = s.pts = lib.av_rescale_q(first_pts, lib.AV_TIME_BASE_Q, inlink.contents.time_base)
                print(f"Set first pts to (in:{s.first_pts} out:{lib.av_rescale_q(first_pts, lib.AV_TIME_BASE_Q, outlink.contents.time_base)})")
            else:
                s.first_pts = s.pts = buf.contents.pts
        else:
            print("Discarding initial frame(s) with no timestamp.")
            lib.av_frame_free(ctypes.byref(buf))
            s.drop += 1
        return 0
    if buf.contents.pts == -9223372036854775808:
        return write_to_fifo(s.fifo, buf)
    delta = lib.av_rescale_q_rnd(buf.contents.pts - s.pts, inlink.contents.time_base, outlink.contents.time_base, s.rounding)
    if delta < 1:
        drop = lib.av_fifo_size(s.fifo) // ctypes.sizeof(ctypes.c_void_p)
        print(f"Dropping {drop} frame(s).")
        s.drop += drop
        tmp = ctypes.c_void_p()
        lib.av_fifo_generic_read(s.fifo, byref(tmp), ctypes.sizeof(tmp), None)
        flush_fifo(s.fifo)
        ret = write_to_fifo(s.fifo, tmp)
        lib.av_frame_free(ctypes.byref(buf))
        return ret
    for i in range(delta):
        buf_out = ctypes.c_void_p()
        lib.av_fifo_generic_read(s.fifo, byref(buf_out), ctypes.sizeof(buf_out), None)
        if not lib.av_fifo_size(s.fifo) and i < delta - 1:
            dup = lib.av_frame_clone(buf_out)
            print("Duplicating frame.")
            if dup:
                ret = write_to_fifo(s.fifo, dup)
            else:
                ret = -12  # AVERROR(ENOMEM)
            if ret < 0:
                lib.av_frame_free(ctypes.byref(buf_out))
                lib.av_frame_free(ctypes.byref(buf))
                return ret
            s.dup += 1
        buf_out.contents.pts = lib.av_rescale_q(s.first_pts, inlink.contents.time_base, outlink.contents.time_base) + s.frames_out
        if (ret := lib.ff_filter_frame(outlink.contents, buf_out)) < 0:
            lib.av_frame_free(ctypes.byref(buf))
            return ret
        s.frames_out += 1
    flush_fifo(s.fifo)
    ret = write_to_fifo(s.fifo, buf)
    s.pts = s.first_pts + lib.av_rescale_q(s.frames_out, outlink.contents.time_base, inlink.contents.time_base)
    return ret