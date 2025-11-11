__revision__ = "$Id$"

__all__ = ['generate', 'construct', 'error', 'ElGamalobj']

from Crypto.PublicKey.pubkey import *
from Crypto.Util import number

class error (Exception):
    pass

def generate(bits, randfunc, progress_func=None):
    obj=ElGamalobj()
    state = 0
    while True:
        if state == 0:
            if progress_func:
                progress_func('p\n')
            q = bignum(getPrime(bits-1, randfunc))
            obj.p = 2*q+1
            if number.isPrime(obj.p, randfunc=randfunc):
                state = 1
            continue
        elif state == 1:
            if progress_func:
                progress_func('g\n')
            obj.g = number.getRandomRange(3, obj.p, randfunc)
            safe = 1
            if pow(obj.g, 2, obj.p)==1:
                safe=0
            if safe and pow(obj.g, q, obj.p)==1:
                safe=0
            if safe and divmod(obj.p-1, obj.g)[1]==0:
                safe=0
            ginv = number.inverse(obj.g, obj.p)
            if safe and divmod(obj.p-1, ginv)[1]==0:
                safe=0
            if safe:
                state = 2
            continue
        elif state == 2:
            if progress_func:
                progress_func('x\n')
            obj.x=number.getRandomRange(2, obj.p-1, randfunc)
            state = 3
            continue
        elif state == 3:
            if progress_func:
                progress_func('y\n')
            obj.y = pow(obj.g, obj.x, obj.p)
            break
    return obj

def construct(tup):
    obj=ElGamalobj()
    if len(tup) not in [3,4]:
        raise ValueError('argument for construct() wrong length')
    for i in range(len(tup)):
        field = obj.keydata[i]
        setattr(obj, field, tup[i])
    return obj

class ElGamalobj(pubkey):
    keydata=['p', 'g', 'y', 'x']

    def encrypt(self, plaintext, K):
        return pubkey.encrypt(self, plaintext, K)
 
    def decrypt(self, ciphertext):
        return pubkey.decrypt(self, ciphertext)

    def sign(self, M, K):
        return pubkey.sign(self, M, K)

    def verify(self, M, signature):
        return pubkey.verify(self, M, signature)

    def _encrypt(self, M, K):
        a=pow(self.g, K, self.p)
        b=( M*pow(self.y, K, self.p) ) % self.p
        return ( a,b )

    def _decrypt(self, M):
        if (not hasattr(self, 'x')):
            raise TypeError('Private key not available in this object')
        ax=pow(M[0], self.x, self.p)
        plaintext=(M[1] * inverse(ax, self.p ) ) % self.p
        return plaintext

    def _sign(self, M, K):
        if (not hasattr(self, 'x')):
            raise TypeError('Private key not available in this object')
        p1=self.p-1
        if (GCD(K, p1)!=1):
            raise ValueError('Bad K value: GCD(K,p-1)!=1')
        a=pow(self.g, K, self.p)
        t=(M-self.x*a) % p1
        while t<0: t=t+p1
        b=(t*inverse(K, p1)) % p1
        return (a, b)

    def _verify(self, M, sig):
        if sig[0]<1 or sig[0]>self.p-1:
            return 0
        v1=pow(self.y, sig[0], self.p)
        v1=(v1*pow(sig[0], sig[1], self.p)) % self.p
        v2=pow(self.g, M, self.p)
        if v1==v2:
            return 1
        return 0

    def size(self):
        return number.size(self.p) - 1

    def has_private(self):
        if hasattr(self, 'x'):
            return 1
        else:
            return 0

    def publickey(self):
        return construct((self.p, self.g, self.y))

object=ElGamalobj