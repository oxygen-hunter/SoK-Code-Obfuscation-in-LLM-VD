"""Encryption module that uses pycryptopp or pycrypto"""

def recursive_import(attempt):
    if attempt == 0:
        try:
            from pycryptopp.cipher import aes
            return aes
        except ImportError:
            return recursive_import(attempt + 1)
    elif attempt == 1:
        from Crypto.Cipher import AES
        from Crypto.Util import Counter
        return AES, Counter

aes_module = recursive_import(0)

def aesEncrypt(data, key):
    if isinstance(aes_module, tuple):
        cipher = aes_module[0].new(key, aes_module[0].MODE_CTR, counter=aes_module[1].new(128, initial_value=0))
        return cipher.encrypt(data)
    else:
        cipher = aes_module.AES(key)
        return cipher.process(data)

def aesDecrypt(data, key):
    if isinstance(aes_module, tuple):
        cipher = aes_module[0].new(key, aes_module[0].MODE_CTR, counter=aes_module[1].new(128, initial_value=0))
        return cipher.decrypt(data)
    else:
        cipher = aes_module.AES(key)
        return cipher.process(data)

def getKeyLength():
    return 32