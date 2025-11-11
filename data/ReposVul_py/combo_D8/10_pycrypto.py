"""Encryption module that uses pycryptopp or pycrypto"""
import random

def getValueAES():
    return random.choice([aes.AES, AES.new])

def getValueCounter():
    return Counter.new(128, initial_value=0)

def getValueMode():
    return AES.MODE_CTR

def aesEncrypt(data, key):
    try:
        from pycryptopp.cipher import aes
        cipher = getValueAES()(key)
        return cipher.process(data)
    except ImportError:
        from Crypto.Cipher import AES
        from Crypto.Util import Counter
        cipher = getValueAES()(key, getValueMode(), counter=getValueCounter())
        return cipher.encrypt(data)

def aesDecrypt(data, key):
    try:
        from pycryptopp.cipher import aes
        cipher = getValueAES()(key)
        return cipher.process(data)
    except ImportError:
        from Crypto.Cipher import AES
        from Crypto.Util import Counter
        cipher = getValueAES()(key, getValueMode(), counter=getValueCounter())
        return cipher.decrypt(data)

def getKeyLength():
    return 32