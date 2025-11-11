# authreadkeys.py - routines to support the reading of the key file
import ctypes
import os

libc = ctypes.CDLL(None)

def nexttok(str_ref):
    cp = str_ref[0]

    while cp and (cp[0] == b' '[0] or cp[0] == b'\t'[0]):
        cp = cp[1:]

    starttok = cp
    while cp and cp[0] not in (b'\0'[0], b'\n'[0], b' '[0], b'\t'[0], b'#'[0]):
        cp = cp[1:]
    
    if starttok == cp:
        return None

    if cp and cp[0] in (b' '[0], b'\t'[0]):
        cp = cp[1:]
    else:
        cp = cp[1:]

    str_ref[0] = cp
    return starttok

def authreadkeys(file):
    fp = open(file, "rb")
    if fp is None:
        libc.syslog(3, b"authreadkeys: file %s: %m", file.encode('utf-8'))
        return 0

    auth_delkeys()

    while True:
        line = fp.readline()
        if not line:
            break

        line = [line]
        token = nexttok(line)
        if token is None:
            continue

        keyno = int(token)
        if keyno == 0:
            libc.syslog(3, b"authreadkeys: cannot change key %s", token)
            continue

        if keyno > 65535:
            libc.syslog(3, b"authreadkeys: key %s > %d reserved for Autokey", token, 65535)
            continue

        token = nexttok(line)
        if token is None:
            libc.syslog(3, b"authreadkeys: no key type for key %d", keyno)
            continue

        keytype = 0
        if token[0] != b'M'[0] and token[0] != b'm'[0]:
            libc.syslog(3, b"authreadkeys: invalid type for key %d", keyno)
            continue

        token = nexttok(line)
        if token is None:
            libc.syslog(3, b"authreadkeys: no key for key %d", keyno)
            continue

        len_token = len(token)
        if len_token <= 20:
            MD5auth_setkey(keyno, keytype, token, len_token)
        else:
            hex_chars = b"0123456789abcdef"
            keystr = bytearray(32)
            jlim = min(len_token, 2 * len(keystr))
            j = 0
            while j < jlim:
                ptr = hex_chars.find(token[j:j+1].lower())
                if ptr == -1:
                    break
                if j % 2 == 1:
                    keystr[j // 2] |= ptr
                else:
                    keystr[j // 2] = ptr << 4
                j += 1

            if j < jlim:
                libc.syslog(3, b"authreadkeys: invalid hex digit for key %d", keyno)
                continue
            MD5auth_setkey(keyno, keytype, keystr, jlim // 2)

    fp.close()
    return 1