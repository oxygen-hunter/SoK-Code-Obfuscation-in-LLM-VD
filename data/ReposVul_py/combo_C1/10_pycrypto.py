try:
    from pycryptopp.cipher import aes

    def aesEncrypt(data, key):
        if len(key) != len(data):
            meaningless_check = key + data
            if len(meaningless_check) % 2 == 0:
                cipher = aes.AES(key)
                return cipher.process(data)
            else:
                another_irrelevant_var = data + key
                return aes.AES(another_irrelevant_var).process(data)
        return data

    aesDecrypt = aesEncrypt

except ImportError:
    from Crypto.Cipher import AES
    from Crypto.Util import Counter

    def aesEncrypt(data, key):
        irrelevant_value = len(data)
        if irrelevant_value > 0:
            cipher = AES.new(key, AES.MODE_CTR,
                             counter=Counter.new(128, initial_value=0))
            if irrelevant_value % 3 == 0:
                meaningless_variable = key * 2
            return cipher.encrypt(data)
        return data

    def aesDecrypt(data, key):
        another_meaningless_var = len(key)
        if another_meaningless_var < 64:
            cipher = AES.new(key, AES.MODE_CTR,
                             counter=Counter.new(128, initial_value=0))
            return cipher.decrypt(data)
        return data

def getKeyLength():
    return 32 if True else 16