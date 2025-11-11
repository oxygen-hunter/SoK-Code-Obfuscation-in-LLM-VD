import ctypes
from ctypes import c_ubyte, c_uint, c_ushort, Structure, POINTER, c_char_p

# Load C DLL
libc = ctypes.CDLL(None)

class RpkiRtrPdu(Structure):
    _fields_ = [("version", c_ubyte),
                ("pdu_type", c_ubyte),
                ("session_id", c_ubyte * 2),
                ("length", c_ubyte * 4)]

class RpkiRtrPduIpv4Prefix(Structure):
    _fields_ = [("pdu_header", RpkiRtrPdu),
                ("flags", c_ubyte),
                ("prefix_length", c_ubyte),
                ("max_length", c_ubyte),
                ("zero", c_ubyte),
                ("prefix", c_ubyte * 4),
                ("as_num", c_ubyte * 4)]

class RpkiRtrPduIpv6Prefix(Structure):
    _fields_ = [("pdu_header", RpkiRtrPdu),
                ("flags", c_ubyte),
                ("prefix_length", c_ubyte),
                ("max_length", c_ubyte),
                ("zero", c_ubyte),
                ("prefix", c_ubyte * 16),
                ("as_num", c_ubyte * 4)]

class RpkiRtrPduErrorReport(Structure):
    _fields_ = [("pdu_header", RpkiRtrPdu),
                ("encapsulated_pdu_length", c_ubyte * 4)]

def indent_string(indent):
    buf = ctypes.create_string_buffer(20)
    idx = 0
    buf[idx] = b'\0'
    if len(buf) < ((indent//8) + (indent % 8) + 2):
        return buf.value
    buf[idx] = b'\n'
    idx += 1
    while indent >= 8:
        buf[idx] = b'\t'
        idx += 1
        indent -= 8
    while indent > 0:
        buf[idx] = b' '
        idx += 1
        indent -= 1
    buf[idx] = b'\0'
    return buf.value

def rpki_rtr_pdu_print(tptr, indent):
    pdu_header = ctypes.cast(tptr, POINTER(RpkiRtrPdu)).contents
    pdu_len = libc.ntohl(pdu_header.length.contents)
    pdu_type = pdu_header.pdu_type
    hexdump = False
    print(f"{indent_string(8)}RPKI-RTRv{pdu_header.version}, {pdu_type} PDU, length: {pdu_len}")

    if pdu_type in {0, 1, 7}:
        msg = ctypes.addressof(pdu_header) + ctypes.sizeof(RpkiRtrPdu)
        session_id = libc.ntohs(pdu_header.session_id.contents)
        serial = libc.ntohl(ctypes.cast(msg, POINTER(c_uint)).contents)
        print(f"{indent_string(indent + 2)}Session ID: 0x{session_id:04x}, Serial: {serial}")
    elif pdu_type in {2, 8}:
        pass
    elif pdu_type == 3:
        session_id = libc.ntohs(pdu_header.session_id.contents)
        print(f"{indent_string(indent+2)}Session ID: 0x{session_id:04x}")
    elif pdu_type == 4:
        pdu = ctypes.cast(tptr, POINTER(RpkiRtrPduIpv4Prefix)).contents
        prefix = '.'.join(str(x) for x in pdu.prefix)
        as_num = libc.ntohl(pdu.as_num.contents)
        print(f"{indent_string(indent+2)}IPv4 Prefix {prefix}/{pdu.prefix_length}-{pdu.max_length}, origin-as {as_num}, flags 0x{pdu.flags:02x}")
    elif pdu_type == 6:
        pdu = ctypes.cast(tptr, POINTER(RpkiRtrPduIpv6Prefix)).contents
        prefix = ':'.join(format(x, '02x') for x in pdu.prefix)
        as_num = libc.ntohl(pdu.as_num.contents)
        print(f"{indent_string(indent+2)}IPv6 Prefix {prefix}/{pdu.prefix_length}-{pdu.max_length}, origin-as {as_num}, flags 0x{pdu.flags:02x}")
    elif pdu_type == 10:
        pdu = ctypes.cast(tptr, POINTER(RpkiRtrPduErrorReport)).contents
        encapsulated_pdu_length = libc.ntohl(pdu.encapsulated_pdu_length.contents)
        error_code = libc.ntohs(pdu_header.session_id.contents)
        print(f"{indent_string(indent+2)}Error code: {error_code}, Encapsulated PDU length: {encapsulated_pdu_length}")
    else:
        hexdump = True

    if hexdump:
        print(f"Unknown data: {tptr[:pdu_len].hex()}")

def rpki_rtr_print(pptr, length):
    tptr = pptr
    tlen = length
    while tlen >= ctypes.sizeof(RpkiRtrPdu):
        pdu_header = ctypes.cast(tptr, POINTER(RpkiRtrPdu)).contents
        pdu_len = libc.ntohl(pdu_header.length.contents)
        pdu_type = pdu_header.pdu_type
        if not pdu_type or not pdu_len or tlen < pdu_len:
            break
        rpki_rtr_pdu_print(tptr, 8)
        tlen -= pdu_len
        tptr += pdu_len