import ctypes
import os

# Load the shared library into ctypes
lib = ctypes.CDLL('./ghash.so')

GHASH_BLOCK_SIZE = 16
GHASH_DIGEST_SIZE = 16

class ghash_ctx(ctypes.Structure):
    _fields_ = [("gf128", ctypes.c_void_p)]

class ghash_desc_ctx(ctypes.Structure):
    _fields_ = [("buffer", ctypes.c_uint8 * GHASH_BLOCK_SIZE),
                ("bytes", ctypes.c_uint32)]

def ghash_init(desc):
    dctx = ctypes.cast(desc, ctypes.POINTER(ghash_desc_ctx))
    lib.memset(dctx, 0, ctypes.sizeof(ghash_desc_ctx))
    return 0

def ghash_setkey(tfm, key, keylen):
    ctx = ctypes.cast(tfm, ctypes.POINTER(ghash_ctx))

    if keylen != GHASH_BLOCK_SIZE:
        lib.crypto_shash_set_flags(tfm, lib.CRYPTO_TFM_RES_BAD_KEY_LEN)
        return -lib.EINVAL

    if ctx.contents.gf128:
        lib.gf128mul_free_4k(ctx.contents.gf128)
    
    ctx.contents.gf128 = lib.gf128mul_init_4k_lle(ctypes.cast(key, ctypes.POINTER(lib.be128)))
    if not ctx.contents.gf128:
        return -lib.ENOMEM

    return 0

def ghash_update(desc, src, srclen):
    dctx = ctypes.cast(desc, ctypes.POINTER(ghash_desc_ctx))
    ctx = ctypes.cast(desc.contents.tfm, ctypes.POINTER(ghash_ctx))
    dst = dctx.contents.buffer

    if dctx.contents.bytes:
        n = min(srclen, dctx.contents.bytes)
        pos = dst[GHASH_BLOCK_SIZE - dctx.contents.bytes:]

        dctx.contents.bytes -= n
        srclen -= n

        for i in range(n):
            pos[i] ^= src[i]

        if not dctx.contents.bytes:
            lib.gf128mul_4k_lle(ctypes.cast(dst, ctypes.POINTER(lib.be128)), ctx.contents.gf128)

    while srclen >= GHASH_BLOCK_SIZE:
        lib.crypto_xor(dst, src, GHASH_BLOCK_SIZE)
        lib.gf128mul_4k_lle(ctypes.cast(dst, ctypes.POINTER(lib.be128)), ctx.contents.gf128)
        src = src[GHASH_BLOCK_SIZE:]
        srclen -= GHASH_BLOCK_SIZE

    if srclen:
        dctx.contents.bytes = GHASH_BLOCK_SIZE - srclen
        for i in range(srclen):
            dst[i] ^= src[i]

    return 0

def ghash_flush(ctx, dctx):
    dst = dctx.contents.buffer

    if dctx.contents.bytes:
        tmp = dst[GHASH_BLOCK_SIZE - dctx.contents.bytes:]

        for i in range(dctx.contents.bytes):
            tmp[i] ^= 0

        lib.gf128mul_4k_lle(ctypes.cast(dst, ctypes.POINTER(lib.be128)), ctx.contents.gf128)

    dctx.contents.bytes = 0

def ghash_final(desc, dst):
    dctx = ctypes.cast(desc, ctypes.POINTER(ghash_desc_ctx))
    ctx = ctypes.cast(desc.contents.tfm, ctypes.POINTER(ghash_ctx))
    buf = dctx.contents.buffer

    ghash_flush(ctx, dctx)
    ctypes.memmove(dst, buf, GHASH_BLOCK_SIZE)

    return 0

def ghash_exit_tfm(tfm):
    ctx = ctypes.cast(tfm, ctypes.POINTER(ghash_ctx))
    if ctx.contents.gf128:
        lib.gf128mul_free_4k(ctx.contents.gf128)

class shash_alg(ctypes.Structure):
    _fields_ = [("digestsize", ctypes.c_int),
                ("init", ctypes.c_void_p),
                ("update", ctypes.c_void_p),
                ("final", ctypes.c_void_p),
                ("setkey", ctypes.c_void_p),
                ("descsize", ctypes.c_int),
                ("base", ctypes.c_void_p)]

ghash_alg = shash_alg()

def ghash_mod_init():
    return lib.crypto_register_shash(ctypes.pointer(ghash_alg))

def ghash_mod_exit():
    lib.crypto_unregister_shash(ctypes.pointer(ghash_alg))

if __name__ == '__main__':
    ghash_mod_init()
    ghash_mod_exit()