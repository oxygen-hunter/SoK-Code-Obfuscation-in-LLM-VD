import ctypes
from ctypes import c_size_t, c_char_p, c_int
import sys

lib = ctypes.CDLL('./asn1_decoder.so')

lib.asn1_find_indefinite_length.argtypes = [ctypes.POINTER(ctypes.c_ubyte), c_size_t, ctypes.POINTER(c_size_t), ctypes.POINTER(c_size_t), ctypes.POINTER(c_char_p)]
lib.asn1_find_indefinite_length.restype = c_int

def asn1_find_indefinite_length(data, datalen, _dp, _len, _errmsg):
    data_array = (ctypes.c_ubyte * len(data))(*data)
    dp = c_size_t(_dp)
    length = c_size_t(_len)
    errmsg = c_char_p(_errmsg.encode('utf-8'))
    result = lib.asn1_find_indefinite_length(data_array, datalen, ctypes.byref(dp), ctypes.byref(length), ctypes.byref(errmsg))
    return result, dp.value, length.value, errmsg.value.decode('utf-8')

def asn1_ber_decoder(decoder, context, data, datalen):
    machine = decoder['machine']
    actions = decoder['actions']
    machlen = decoder['machlen']

    # Python equivalent for enum
    ASN1_OP_COMPLETE = 14
    ASN1_OP_RETURN = 17

    # Initialize variables
    op = 0
    tag = 0
    csp = 0
    jsp = 0
    optag = 0
    hdr = 0
    flags = 0
    pc = 0
    dp = 0
    tdp = 0
    length = 0
    errmsg = ''
    ret = 0

    NR_CONS_STACK = 10
    NR_JUMP_STACK = 10

    cons_dp_stack = [0] * NR_CONS_STACK
    cons_datalen_stack = [0] * NR_CONS_STACK
    cons_hdrlen_stack = [0] * NR_CONS_STACK
    jump_stack = [0] * NR_JUMP_STACK

    if datalen > 65535:
        return -sys.getsizeof("EMSGSIZE")
    
    # Define flags
    FLAG_INDEFINITE_LENGTH = 0x01
    FLAG_MATCHED = 0x02
    FLAG_LAST_MATCHED = 0x04
    FLAG_CONS = 0x20

    while True:
        if pc >= machlen:
            errmsg = "Machine overrun error"
            break
        if pc + decoder['op_lengths'][op] > machlen:
            errmsg = "Machine overrun error"
            break

        op = machine[pc]

        if op <= ASN1_OP_COMPLETE:
            if (op & FLAG_MATCHED) and flags & FLAG_MATCHED:
                pc += decoder['op_lengths'][op]
                continue

            flags = 0
            hdr = 2

            if dp >= datalen - 1:
                errmsg = "Data overrun error"
                break

            tag = data[dp]
            dp += 1

            if (tag & 0x1f) == decoder['LONG_TAG']:
                errmsg = "Long tag not supported"
                break

            optag = machine[pc + 1]
            flags |= optag & FLAG_CONS

            tmp = optag ^ tag
            tmp &= ~(optag & decoder['CONS_BIT'])

            if tmp != 0:
                if op & FLAG_MATCHED:
                    pc += decoder['op_lengths'][op]
                    dp -= 1
                    continue
                errmsg = "Unexpected tag"
                break

            flags |= FLAG_MATCHED
            length = data[dp]
            dp += 1

            if length > 0x7f:
                if length == decoder['INDEFINITE_LENGTH']:
                    if not (tag & decoder['CONS_BIT']):
                        errmsg = "Indefinite len primitive not permitted"
                        break
                    flags |= FLAG_INDEFINITE_LENGTH
                    if 2 > datalen - dp:
                        errmsg = "Data overrun error"
                        break
                else:
                    n = length - 0x80
                    if n > 2:
                        errmsg = "Unsupported length"
                        break
                    if dp >= datalen - n:
                        errmsg = "Data overrun error"
                        break
                    hdr += n
                    for _ in range(n):
                        length <<= 8
                        length |= data[dp]
                        dp += 1
                    if length > datalen - dp:
                        errmsg = "Data overrun error"
                        break

            if flags & FLAG_CONS:
                if csp >= NR_CONS_STACK:
                    errmsg = "Cons stack overflow"
                    break
                cons_dp_stack[csp] = dp
                cons_hdrlen_stack[csp] = hdr
                if not (flags & FLAG_INDEFINITE_LENGTH):
                    cons_datalen_stack[csp] = datalen
                    datalen = dp + length
                else:
                    cons_datalen_stack[csp] = 0
                csp += 1

            tdp = dp

        if op == ASN1_OP_COMPLETE:
            if jsp != 0 or csp != 0:
                errmsg = "Stacks not empty at completion"
                break
            return 0

        if op == ASN1_OP_RETURN:
            if jsp <= 0:
                errmsg = "Jump stack underflow"
                break
            pc = jump_stack[jsp - 1]
            jsp -= 1
            flags |= FLAG_MATCHED | FLAG_LAST_MATCHED
            continue

    print(f"ASN1: {errmsg} [m={pc} d={dp} ot={optag} t={tag} l={length}]")
    return -sys.getsizeof("EBADMSG")