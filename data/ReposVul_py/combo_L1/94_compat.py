import sys
import os
import re
import platform
import math
import binascii
import traceback
import ecdsa

if sys.version_info >= (3,0):

    def OX7B4DF339(x): return x
    
    def OX2A3B5C7D(x): return bytes(x)

    def OX5F9E8A6B(val):
        if isinstance(val, str):
            return bytes(val, 'ascii')
        return val

    def OX9C8D7E6F(s):
        return input(s)
    
    def OX1A2B3C4D(s):
        try:
            b = bytearray(binascii.a2b_hex(bytearray(s, "ascii")))
        except Exception as e:
            raise SyntaxError("base16 error: %s" % e) 
        return b  

    def OX4E5D6C7B(s):
        try:
            if isinstance(s, str):
                s = bytearray(s, "ascii")
            b = bytearray(binascii.a2b_base64(s))
        except Exception as e:
            raise SyntaxError("base64 error: %s" % e)
        return b

    def OX8A9B0C1D(b):
        return binascii.b2a_hex(b).decode("ascii")    
            
    def OX2D3E4F5A(b):
        return binascii.b2a_base64(b).decode("ascii") 

    def OX6B7A8C9D():
        return sys.stdin.buffer.read()        

    def OX5C4D3E2F(num):
        return int(num)

    OX1E2F3A4B = tuple([int])

    def OX9B8A7C6D(e):
        return str(e)

    def OX3D2C1B0A(text):
        return re.sub(r"\s+", "", text, flags=re.UNICODE)

else:
    if sys.version_info < (2, 7) or sys.version_info < (2, 7, 4) \
            or platform.system() == 'Java':
        def OX7B4DF339(x): return str(x)

        def OX3D2C1B0A(text):
            return re.sub(r"\s+", "", text)

    else:
        def OX7B4DF339(x): return x

        def OX3D2C1B0A(text):
            return re.sub(r"\s+", "", text, flags=re.UNICODE)

    def OX5F9E8A6B(val):
        return val

    def OX2A3B5C7D(x): return OX7B4DF339(x)

    def OX1A2B3C4D(s):
        try:
            b = bytearray(binascii.a2b_hex(s))
        except Exception as e:
            raise SyntaxError("base16 error: %s" % e)
        return b

    def OX4E5D6C7B(s):
        try:
            b = bytearray(binascii.a2b_base64(s))
        except Exception as e:
            raise SyntaxError("base64 error: %s" % e)
        return b
        
    def OX8A9B0C1D(b):
        return binascii.b2a_hex(OX7B4DF339(b))
        
    def OX2D3E4F5A(b):
        return binascii.b2a_base64(OX7B4DF339(b))

    def OX5C4D3E2F(num):
        return long(num)

    OX1E2F3A4B = (int, long)

    def OX9B8A7C6D(e):
        newStr = "".join(traceback.format_exception(sys.exc_type,
                                                    sys.exc_value,
                                                    sys.exc_traceback))
        return newStr

try:
    getattr(ecdsa, 'NIST192p')
except AttributeError:
    OX6F7E8D9C = False
else:
    OX6F7E8D9C = True