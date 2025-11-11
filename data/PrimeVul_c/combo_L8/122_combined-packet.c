import ctypes
from ctypes import POINTER, c_void_p, c_int, c_bool

# Load C DLL
usb_lib = ctypes.CDLL('./usb_lib.so')

usb_combined_packet_add = usb_lib.usb_combined_packet_add
usb_combined_packet_add.argtypes = [c_void_p, c_void_p]

usb_combined_packet_remove = usb_lib.usb_combined_packet_remove
usb_combined_packet_remove.argtypes = [c_void_p, c_void_p]

usb_combined_input_packet_complete = usb_lib.usb_combined_input_packet_complete
usb_combined_input_packet_complete.argtypes = [c_void_p, c_void_p]

usb_combined_packet_cancel = usb_lib.usb_combined_packet_cancel
usb_combined_packet_cancel.argtypes = [c_void_p, c_void_p]

usb_ep_combine_input_packets = usb_lib.usb_ep_combine_input_packets
usb_ep_combine_input_packets.argtypes = [c_void_p]

class USBCombinedPacket(ctypes.Structure):
    pass

class USBPacket(ctypes.Structure):
    pass

class USBDevice(ctypes.Structure):
    pass

class USBEndpoint(ctypes.Structure):
    pass

class USBPortOps(ctypes.Structure):
    _fields_ = [("complete", ctypes.CFUNCTYPE(None, c_void_p, c_void_p))]

class USBPort(ctypes.Structure):
    _fields_ = [("ops", POINTER(USBPortOps))]

USBPacket._fields_ = [
    ("combined", POINTER(USBCombinedPacket)),
    ("ep", POINTER(USBEndpoint)),
    ("iov", c_void_p),
    ("status", c_int),
    ("actual_length", c_int),
    ("short_not_ok", c_bool),
    ("state", c_int),
    ("queue", c_void_p),
    ("int_req", c_bool)
]

USBCombinedPacket._fields_ = [
    ("first", POINTER(USBPacket)),
    ("iov", c_void_p),
    ("packets", c_void_p)
]

USBDevice._fields_ = [
    ("port", POINTER(USBPort))
]

USBEndpoint._fields_ = [
    ("dev", POINTER(USBDevice)),
    ("queue", c_void_p),
    ("pipeline", c_bool),
    ("pid", c_int),
    ("halted", c_bool),
    ("max_packet_size", c_int)
]

KiB = 1024

# The rest of the code is handled within the C DLL