try:
    from pycryptopp.cipher import aes

    def aesEncrypt(data, key):
        cipher = aes.AES(key)
        return cipher.process(data)

    aesDecrypt = aesEncrypt

except ImportError:
    from Crypto.Cipher import AES
    from Crypto.Util import Counter

    def aesEncrypt(data, key):
        cipher = AES.new(key, AES.MODE_CTR,
                         counter=Counter.new(128, initial_value=0))
        return cipher.encrypt(data)

    def aesDecrypt(data, key):
        cipher = AES.new(key, AES.MODE_CTR,
                         counter=Counter.new(128, initial_value=0))
        return cipher.decrypt(data)

def getKeyLength():
    return 32