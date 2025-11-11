import ctypes
import os

# Load the C shared library
libcrypto = ctypes.CDLL('./libcrypto.so')

# Define types and functions
libcrypto.crypto_default_rng_lock = ctypes.CFUNCTYPE(None)()
libcrypto.crypto_default_rng_refcnt = ctypes.c_int.in_dll(libcrypto, 'crypto_default_rng_refcnt')
crypto_default_rng = ctypes.c_void_p.in_dll(libcrypto, 'crypto_default_rng')

def generate(tfm, src, slen, dst, dlen):
    return libcrypto.generate(ctypes.byref(tfm), ctypes.byref(src), slen, ctypes.byref(dst), dlen)

def rngapi_reset(tfm, seed, slen):
    buf = ctypes.POINTER(ctypes.c_ubyte)()
    src = ctypes.cast(seed, ctypes.POINTER(ctypes.c_ubyte))
    err = 0

    if slen:
        buf = ctypes.cast(libcrypto.kmalloc(slen, ctypes.c_int), ctypes.POINTER(ctypes.c_ubyte))
        if not buf:
            return -ctypes.get_errno()

        ctypes.memmove(buf, seed, slen)
        src = buf

    err = libcrypto.rng_reset(ctypes.byref(tfm), ctypes.byref(src), slen)

    libcrypto.kzfree(buf)
    return err

def crypto_rng_reset(tfm, seed, slen):
    buf = ctypes.POINTER(ctypes.c_ubyte)()
    err = 0

    if not seed and slen:
        buf = ctypes.cast(libcrypto.kmalloc(slen, ctypes.c_int), ctypes.POINTER(ctypes.c_ubyte))
        if not buf:
            return -ctypes.get_errno()

        libcrypto.get_random_bytes(ctypes.byref(buf), slen)
        seed = buf

    err = libcrypto.seed(ctypes.byref(tfm), ctypes.byref(seed), slen)

    libcrypto.kfree(buf)
    return err

def crypto_get_default_rng():
    rng = ctypes.c_void_p()
    err = 0

    libcrypto.mutex_lock(libcrypto.crypto_default_rng_lock)
    if not crypto_default_rng:
        rng = libcrypto.crypto_alloc_rng(b"stdrng", 0, 0)
        err = ctypes.cast(rng, ctypes.c_int).value
        if libcrypto.IS_ERR(rng):
            goto unlock

        err = crypto_rng_reset(rng, None, libcrypto.crypto_rng_seedsize(rng))
        if err:
            libcrypto.crypto_free_rng(rng)
            goto unlock

        crypto_default_rng.value = rng

    libcrypto.crypto_default_rng_refcnt.value += 1
    err = 0

    unlock:
    libcrypto.mutex_unlock(libcrypto.crypto_default_rng_lock)
    return err

def crypto_put_default_rng():
    libcrypto.mutex_lock(libcrypto.crypto_default_rng_lock)
    if not ctypes.c_int(libcrypto.crypto_default_rng_refcnt.value - 1):
        libcrypto.crypto_free_rng(crypto_default_rng)
        crypto_default_rng.value = None
    libcrypto.mutex_unlock(libcrypto.crypto_default_rng_lock)

def crypto_register_rng(alg):
    base = ctypes.byref(alg.base)

    if alg.seedsize > libcrypto.PAGE_SIZE // 8:
        return -libcrypto.EINVAL

    base.contents.cra_type = ctypes.byref(libcrypto.crypto_rng_type)
    base.contents.cra_flags &= ~libcrypto.CRYPTO_ALG_TYPE_MASK
    base.contents.cra_flags |= libcrypto.CRYPTO_ALG_TYPE_RNG

    return libcrypto.crypto_register_alg(base)

def crypto_unregister_rng(alg):
    libcrypto.crypto_unregister_alg(ctypes.byref(alg.base))

def crypto_register_rngs(algs, count):
    i, ret = 0, 0
    for i in range(count):
        ret = crypto_register_rng(algs + i)
        if ret:
            goto err

    return 0

    err:
    for i in range(i - 1, -1, -1):
        crypto_unregister_rng(algs + i)

    return ret

def crypto_unregister_rngs(algs, count):
    for i in range(count - 1, -1, -1):
        crypto_unregister_rng(algs + i)

# Module license and description
libcrypto.MODULE_LICENSE(b"GPL")
libcrypto.MODULE_DESCRIPTION(b"Random Number Generator")