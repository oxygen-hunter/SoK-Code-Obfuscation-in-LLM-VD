import ctypes
import os

# Load the C library
libname = os.path.abspath("librl2.so")  # Assume the shared library is named librl2.so
rl2lib = ctypes.CDLL(libname)

# Define the C function signatures in Python
rl2_probe = rl2lib.rl2_probe
rl2_probe.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
rl2_probe.restype = ctypes.c_int

rl2_read_header = rl2lib.rl2_read_header
rl2_read_header.argtypes = [ctypes.c_void_p]
rl2_read_header.restype = ctypes.c_int

rl2_read_packet = rl2lib.rl2_read_packet
rl2_read_packet.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
rl2_read_packet.restype = ctypes.c_int

rl2_read_seek = rl2lib.rl2_read_seek
rl2_read_seek.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int64, ctypes.c_int]
rl2_read_seek.restype = ctypes.c_int

class AVProbeData(ctypes.Structure):
    _fields_ = [("buf", ctypes.POINTER(ctypes.c_ubyte))]

class AVFormatContext(ctypes.Structure):
    pass

class AVPacket(ctypes.Structure):
    pass

def rl2_demuxer():
    # Example usage in Python
    p = AVProbeData()
    p.buf = (ctypes.c_ubyte * 12)()

    probe_result = rl2_probe(ctypes.byref(p))
    print(f"Probe result: {probe_result}")

    # Assuming s is a valid AVFormatContext pointer
    s = ctypes.pointer(AVFormatContext())
    header_result = rl2_read_header(s)
    print(f"Header read result: {header_result}")

    pkt = AVPacket()
    packet_result = rl2_read_packet(s, ctypes.byref(pkt))
    print(f"Packet read result: {packet_result}")

    seek_result = rl2_read_seek(s, 0, 1000, 0)
    print(f"Seek result: {seek_result}")

if __name__ == "__main__":
    rl2_demuxer()