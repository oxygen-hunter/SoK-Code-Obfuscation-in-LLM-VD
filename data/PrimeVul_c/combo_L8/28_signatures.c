import ctypes
import ctypes.util
import os

# Load the C library
lib = ctypes.cdll.LoadLibrary(ctypes.util.find_library("crypto"))

# C Structures and Functions
class RSA_private_key(ctypes.Structure):
    _fields_ = [("p", ctypes.c_void_p),
                ("dP", ctypes.c_void_p),
                ("q", ctypes.c_void_p),
                ("dQ", ctypes.c_void_p),
                ("qInv", ctypes.c_void_p)]

class private_key_stuff(ctypes.Structure):
    _fields_ = [("u", RSA_private_key)]

class RSA_public_key(ctypes.Structure):
    _fields_ = [("e", ctypes.c_void_p),
                ("n", ctypes.c_void_p)]

def sign_hash(pks, hash_val, hash_len, sig_val, sig_len):
    ch = ctypes.create_string_buffer(sig_len)
    t1 = ctypes.c_void_p()
    padlen = sig_len - 3 - hash_len
    p = ctypes.pointer(ctypes.c_ubyte.from_buffer(sig_val))

    k = pks.contents.u

    # PKCS#1 v1.5 8.1 encryption-block formatting
    p[0] = 0x00
    p[1] = 0x01  # BT (block type) 01
    for i in range(2, 2 + padlen):
        p[i] = 0xFF
    p[padlen + 2] = 0x00
    ctypes.memmove(ctypes.byref(p, padlen + 3), hash_val, hash_len)

    # PKCS#1 v1.5 8.2 octet-string-to-integer conversion
    lib.n_to_mpz(ctypes.byref(t1), sig_val, sig_len)

    # PKCS#1 v1.5 8.3 RSA computation y = x^c mod n
    lib.oswcrypto_rsa_mod_exp_crt(t1, t1, k.p, k.dP, k.q, k.dQ, k.qInv)

    ch = lib.mpz_to_n(t1, sig_len)
    ctypes.memmove(sig_val, ch, sig_len)
    lib.pfree(ch)

    lib.mpz_clear(t1)

def verify_signed_hash(k, s, s_max_octets, psig, hash_len, sig_val, sig_len):
    padlen = sig_len - 3 - hash_len

    # actual exponentiation; see PKCS#1 v2.0 5.1
    temp_s, c = ctypes.create_string_buffer(sig_len), ctypes.c_void_p()

    lib.n_to_mpz(ctypes.byref(c), sig_val, sig_len)
    lib.oswcrypto_mod_exp(ctypes.byref(c), ctypes.byref(c), k.e, k.n)

    temp_s = lib.mpz_to_n(ctypes.byref(c), sig_len)
    if s_max_octets < sig_len:
        return "2exponentiation failed; too many octets"
    ctypes.memmove(s, temp_s, sig_len)
    lib.pfree(temp_s)
    lib.mpz_clear(c)

    # check signature contents
    # verify padding (not including any DER digest info!)
    if s[0] != 0x00 or s[1] != 0x01 or s[padlen + 2] != 0x00:
        return "3SIG padding does not check out"

    psig.contents.value = ctypes.byref(s, padlen + 3)

    return None