import ctypes
from ctypes import cdll, c_uint8, c_uint16, c_uint64, c_void_p

# Load a hypothetical C/C++ shared library
lib = cdll.LoadLibrary('libfizz.so')

class EncryptedReadRecordLayer:
    def __init__(self, encryptionLevel):
        self.encryptionLevel_ = encryptionLevel
        self.seqNum_ = 0
        self.skipFailedDecryption_ = False
        self.aead_ = lib.getAeadInstance()
        self.useAdditionalData_ = True

    def getDecryptedBuf(self, buf):
        while True:
            if not buf or len(buf) < 5:
                return None
            
            ad = buf[:5]
            contentType = ad[0]
            length = int.from_bytes(ad[3:5], 'big')

            if length == 0 or length > 0x4256:
                raise RuntimeError("Invalid length")

            if contentType == 21 and length == 2:
                alert = lib.decodeAlert(buf)
                raise RuntimeError(f"Alert received: {alert}")

            encrypted = buf[5:5+length]
            buf = buf[5+length:]

            if contentType == 20:
                if encrypted == b'\x01':
                    continue
                else:
                    raise Exception("Illegal parameter")

            if self.seqNum_ == 0xFFFFFFFFFFFFFFFF:
                raise RuntimeError("Max read seq num")

            if self.skipFailedDecryption_:
                decryptAttempt = lib.tryDecrypt(encrypted, ad if self.useAdditionalData_ else None, c_uint64(self.seqNum_))
                if decryptAttempt:
                    self.seqNum_ += 1
                    self.skipFailedDecryption_ = False
                    return decryptAttempt
                else:
                    continue
            else:
                return lib.decrypt(encrypted, ad if self.useAdditionalData_ else None, c_uint64(self.seqNum_)).contents

    def read(self, buf):
        decryptedBuf = self.getDecryptedBuf(buf)
        if not decryptedBuf:
            return None

        msg = decryptedBuf
        return msg

    def getEncryptionLevel(self):
        return self.encryptionLevel_

class EncryptedWriteRecordLayer:
    def __init__(self, encryptionLevel):
        self.encryptionLevel_ = encryptionLevel
        self.seqNum_ = 0
        self.aead_ = lib.getAeadInstance()
        self.useAdditionalData_ = True

    def write(self, msg):
        queue = msg.fragment
        outBuf = None

        while queue:
            dataBuf = queue
            queue = b''

            header = bytearray(5)
            header[0] = 23
            header[1] = 3
            header[2] = 3
            ciphertextLength = len(dataBuf) + self.aead_.getCipherOverhead()
            header[3:5] = ciphertextLength.to_bytes(2, 'big')

            cipherText = lib.encrypt(dataBuf, header if self.useAdditionalData_ else None, c_uint64(self.seqNum_))
            self.seqNum_ += 1

            record = header + cipherText

            if outBuf is None:
                outBuf = record
            else:
                outBuf += record

        if outBuf is None:
            outBuf = b''

        return outBuf

    def getEncryptionLevel(self):
        return self.encryptionLevel_