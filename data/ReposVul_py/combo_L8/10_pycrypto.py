import ctypes
from ctypes import cdll, c_char_p, c_void_p, c_size_t

try:
    from pycryptopp.cipher import aes

    def aesEncrypt(data, key):
        cipher = aes.AES(key)
        return cipher.process(data)

    aesDecrypt = aesEncrypt

except ImportError:
    class AES_CTR:
        def __init__(self, key):
            self.aes_lib = cdll.LoadLibrary('libcrypto.so')
            self.key = key

        def encrypt(self, data):
            self.aes_lib.AES_encrypt.argtypes = [c_char_p, c_char_p, c_size_t]
            data_ptr = c_char_p(data)
            key_ptr = c_char_p(self.key)
            out_data = ctypes.create_string_buffer(len(data))
            self.aes_lib.AES_encrypt(data_ptr, key_ptr, c_size_t(len(data)), out_data)
            return out_data.raw

        def decrypt(self, data):
            self.aes_lib.AES_decrypt.argtypes = [c_char_p, c_char_p, c_size_t]
            data_ptr = c_char_p(data)
            key_ptr = c_char_p(self.key)
            out_data = ctypes.create_string_buffer(len(data))
            self.aes_lib.AES_decrypt(data_ptr, key_ptr, c_size_t(len(data)), out_data)
            return out_data.raw

    def aesEncrypt(data, key):
        cipher = AES_CTR(key)
        return cipher.encrypt(data)

    def aesDecrypt(data, key):
        cipher = AES_CTR(key)
        return cipher.decrypt(data)

def getKeyLength():
    return 32