__revision__ = "$Id$"

__all__ = ['OXA57E8F8C', 'OXF4A9D7A0', 'OX0F3D4B9A', 'OX7B4DF339']

from Crypto.PublicKey.pubkey import *
from Crypto.Util import number

class OX0F3D4B9A (Exception):
    pass

def OXA57E8F8C(OX5D6B1C9A, OX3E8E6F1B, OXB4F9C3D0=None):
    OX9E1F2A3B=OX7B4DF339()
    if OXB4F9C3D0:
        OXB4F9C3D0('p\n')
    while 1:
        OX5A4B3C2D = bignum(getPrime(OX5D6B1C9A-1, OX3E8E6F1B))
        OX9E1F2A3B.OX7D3C2B1A = 2*OX5A4B3C2D+1
        if number.isPrime(OX9E1F2A3B.OX7D3C2B1A, randfunc=OX3E8E6F1B):
            break
    if OXB4F9C3D0:
        OXB4F9C3D0('g\n')
    while 1:
        OX9E1F2A3B.OX3B2A1C4D = number.getRandomRange(3, OX9E1F2A3B.OX7D3C2B1A, OX3E8E6F1B)
        OX6E5D4C3B = 1
        if pow(OX9E1F2A3B.OX3B2A1C4D, 2, OX9E1F2A3B.OX7D3C2B1A)==1:
            OX6E5D4C3B=0
        if OX6E5D4C3B and pow(OX9E1F2A3B.OX3B2A1C4D, OX5A4B3C2D, OX9E1F2A3B.OX7D3C2B1A)==1:
            OX6E5D4C3B=0
        if OX6E5D4C3B and divmod(OX9E1F2A3B.OX7D3C2B1A-1, OX9E1F2A3B.OX3B2A1C4D)[1]==0:
            OX6E5D4C3B=0
        OX7C2B1A4D = number.inverse(OX9E1F2A3B.OX3B2A1C4D, OX9E1F2A3B.OX7D3C2B1A)
        if OX6E5D4C3B and divmod(OX9E1F2A3B.OX7D3C2B1A-1, OX7C2B1A4D)[1]==0:
            OX6E5D4C3B=0
        if OX6E5D4C3B:
            break
    if OXB4F9C3D0:
        OXB4F9C3D0('x\n')
    OX9E1F2A3B.OX2A1C3B4D=number.getRandomRange(2, OX9E1F2A3B.OX7D3C2B1A-1, OX3E8E6F1B)
    if OXB4F9C3D0:
        OXB4F9C3D0('y\n')
    OX9E1F2A3B.OX4B3A2C1D = pow(OX9E1F2A3B.OX3B2A1C4D, OX9E1F2A3B.OX2A1C3B4D, OX9E1F2A3B.OX7D3C2B1A)
    return OX9E1F2A3B

def OXF4A9D7A0(OX7E4D3C2B):
    OX9E1F2A3B=OX7B4DF339()
    if len(OX7E4D3C2B) not in [3,4]:
        raise ValueError('argument for construct() wrong length')
    for OX0A9B8C7D in range(len(OX7E4D3C2B)):
        OX1B2C3D4E = OX9E1F2A3B.OX3A2B1C4D[OX0A9B8C7D]
        setattr(OX9E1F2A3B, OX1B2C3D4E, OX7E4D3C2B[OX0A9B8C7D])
    return OX9E1F2A3B

class OX7B4DF339(pubkey):
    OX3A2B1C4D=['OX7D3C2B1A', 'OX3B2A1C4D', 'OX4B3A2C1D', 'OX2A1C3B4D']

    def OX9A8B7C6D(self, OX1A2B3C4D, OX5B4A3C2D):
        return pubkey.encrypt(self, OX1A2B3C4D, OX5B4A3C2D)
 
    def OXB9C8D7A6(self, OX1A2B3C4D):
        return pubkey.decrypt(self, OX1A2B3C4D)

    def OX8A7B6C5D(self, OX3A4B5C6D, OXC9D8E7A6):
        return pubkey.sign(self, OX3A4B5C6D, OXC9D8E7A6)

    def OX7C6B5A4D(self, OX3A4B5C6D, OX9B8A7C6D):
        return pubkey.verify(self, OX3A4B5C6D, OX9B8A7C6D)

    def OXA6B5C4D3(self, OX3A4B5C6D, OX9B8A7C6D):
        OX8E7D6C5B=pow(self.OX3B2A1C4D, OX9B8A7C6D, self.OX7D3C2B1A)
        OX1B2A3C4D=( OX3A4B5C6D*pow(self.OX4B3A2C1D, OX9B8A7C6D, self.OX7D3C2B1A) ) % self.OX7D3C2B1A
        return ( OX8E7D6C5B,OX1B2A3C4D )

    def OX9D8E7F6A(self, OX8E7D6C5B):
        if (not hasattr(self, 'OX2A1C3B4D')):
            raise TypeError('Private key not available in this object')
        OXA9B8C7D=pow(OX8E7D6C5B[0], self.OX2A1C3B4D, self.OX7D3C2B1A)
        OX1B2A3C4D=(OX8E7D6C5B[1] * inverse(OXA9B8C7D, self.OX7D3C2B1A ) ) % self.OX7D3C2B1A
        return OX1B2A3C4D

    def OXC5B4A3D2(self, OX3A4B5C6D, OXC9D8E7A6):
        if (not hasattr(self, 'OX2A1C3B4D')):
            raise TypeError('Private key not available in this object')
        OX1B2A3C4D=self.OX7D3C2B1A-1
        if (GCD(OXC9D8E7A6, OX1B2A3C4D)!=1):
            raise ValueError('Bad K value: GCD(K,p-1)!=1')
        OXA9B8C7D=pow(self.OX3B2A1C4D, OXC9D8E7A6, self.OX7D3C2B1A)
        OX7C8D9E1A=(OX3A4B5C6D-self.OX2A1C3B4D*OXA9B8C7D) % OX1B2A3C4D
        while OX7C8D9E1A<0: OX7C8D9E1A=OX7C8D9E1A+OX1B2A3C4D
        OX6B7A8C9D=(OX7C8D9E1A*inverse(OXC9D8E7A6, OX1B2A3C4D)) % OX1B2A3C4D
        return (OXA9B8C7D, OX6B7A8C9D)

    def OX4E3D2C1B(self, OX3A4B5C6D, OX9B8A7C6D):
        if OX9B8A7C6D[0]<1 or OX9B8A7C6D[0]>OX7D3C2B1A-1:
            return 0
        OX5C4B3A2D=pow(self.OX4B3A2C1D, OX9B8A7C6D[0], self.OX7D3C2B1A)
        OX5C4B3A2D=(OX5C4B3A2D*pow(OX9B8A7C6D[0], OX9B8A7C6D[1], self.OX7D3C2B1A)) % self.OX7D3C2B1A
        OX1E2D3C4B=pow(self.OX3B2A1C4D, OX3A4B5C6D, self.OX7D3C2B1A)
        if OX5C4B3A2D==OX1E2D3C4B:
            return 1
        return 0

    def OX6B5C4A3D(self):
        return number.size(self.OX7D3C2B1A) - 1

    def OX3D4C5B6A(self):
        if hasattr(self, 'OX2A1C3B4D'):
            return 1
        else:
            return 0

    def OX8C7D6B5A(self):
        return OXF4A9D7A0((self.OX7D3C2B1A, self.OX3B2A1C4D, self.OX4B3A2C1D))


object=OX7B4DF339