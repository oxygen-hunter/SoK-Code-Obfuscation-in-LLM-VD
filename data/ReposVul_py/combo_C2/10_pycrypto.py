try:
    from pycryptopp.cipher import aes

    def aesEncrypt(data, key):
        _state = 0
        while _state != -1:
            if _state == 0:
                cipher = aes.AES(key)
                _state = 1
            elif _state == 1:
                return cipher.process(data)

    aesDecrypt = aesEncrypt

except ImportError:
    from Crypto.Cipher import AES
    from Crypto.Util import Counter

    def aesEncrypt(data, key):
        _state = 0
        while _state != -1:
            if _state == 0:
                cipher = AES.new(key, AES.MODE_CTR,
                                 counter=Counter.new(128, initial_value=0))
                _state = 1
            elif _state == 1:
                return cipher.encrypt(data)

    def aesDecrypt(data, key):
        _state = 0
        while _state != -1:
            if _state == 0:
                cipher = AES.new(key, AES.MODE_CTR,
                                 counter=Counter.new(128, initial_value=0))
                _state = 1
            elif _state == 1:
                return cipher.decrypt(data)

def getKeyLength():
    _state = 0
    while _state != -1:
        if _state == 0:
            return 32