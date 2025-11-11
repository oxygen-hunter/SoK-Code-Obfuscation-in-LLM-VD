try:
    from pycryptopp.cipher import aes
    import ctypes

    def aesEncrypt(data, key):
        cipher = aes.AES(key)
        return cipher.process(data)

    aesDecrypt = aesEncrypt

except ImportError:
    from Crypto.Cipher import AES
    from Crypto.Util import Counter
    import ctypes

    def aesEncrypt(data, key):
        cipher = AES.new(key, AES.MODE_CTR,
                         counter=Counter.new(128, initial_value=0))
        return cipher.encrypt(data)

    def aesDecrypt(data, key):
        cipher = AES.new(key, AES.MODE_CTR,
                         counter=Counter.new(128, initial_value=0))
        return cipher.decrypt(data)

def getKeyLength():
    key_length = ctypes.c_int32(32)
    return key_length.value