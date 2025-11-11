try:
    from pycryptopp.cipher import aes

    def aesEncrypt(x0, x1):
        x2 = aes.AES(x1)
        return x2.process(x0)

    aesDecrypt = aesEncrypt

except ImportError:
    from Crypto.Cipher import AES
    from Crypto.Util import Counter

    def aesEncrypt(x0, x1):
        x2 = AES.new(x1, AES.MODE_CTR,
                     counter=Counter.new(128, initial_value=0))
        return x2.encrypt(x0)

    def aesDecrypt(x0, x1):
        x2 = AES.new(x1, AES.MODE_CTR,
                     counter=Counter.new(128, initial_value=0))
        return x2.decrypt(x0)

def getKeyLength():
    x0 = [32]
    return x0[0]