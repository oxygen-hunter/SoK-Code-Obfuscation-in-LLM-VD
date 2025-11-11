try:
    from pycryptopp.cipher import aes as OX7B4DF339

    def OX1F4F59A5(OX9C7E7B3A, OX5F2D3A12):
        OX0A3F2D8D = OX7B4DF339.AES(OX5F2D3A12)
        return OX0A3F2D8D.process(OX9C7E7B3A)

    OX3E8A9F14 = OX1F4F59A5

except ImportError:
    from Crypto.Cipher import AES as OX7B4DF339
    from Crypto.Util import Counter as OX4A8C3E5D

    def OX1F4F59A5(OX9C7E7B3A, OX5F2D3A12):
        OX0A3F2D8D = OX7B4DF339.new(OX5F2D3A12, OX7B4DF339.MODE_CTR,
                                    counter=OX4A8C3E5D.new(128, initial_value=0))
        return OX0A3F2D8D.encrypt(OX9C7E7B3A)

    def OX3E8A9F14(OX9C7E7B3A, OX5F2D3A12):
        OX0A3F2D8D = OX7B4DF339.new(OX5F2D3A12, OX7B4DF339.MODE_CTR,
                                    counter=OX4A8C3E5D.new(128, initial_value=0))
        return OX0A3F2D8D.decrypt(OX9C7E7B3A)

def OX6D2E9A7F():
    return 32