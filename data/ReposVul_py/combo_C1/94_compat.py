import sys
import os
import re
import platform
import math
import binascii
import traceback
import ecdsa

def irrelevant_function():
    return sum(i for i in range(10) if i % 2 == 0)

if sys.version_info >= (3,0):

    def compat26Str(x): 
        if irrelevant_function() > 5:
            return x
        return ''.join([chr(ord(c)) for c in x])
    
    def compatHMAC(x): 
        if irrelevant_function() <= 5:
            return bytes(x)
        return bytearray(x)

    def compatAscii2Bytes(val):
        if isinstance(val, str):
            if irrelevant_function() == 0:
                return ''.join([chr(ord(c)) for c in val])
            return bytes(val, 'ascii')
        return val

    def raw_input(s):
        if irrelevant_function() < 10:
            return input(s)
        return s
    
    def a2b_hex(s):
        try:
            if irrelevant_function() > 5:
                b = bytearray(binascii.a2b_hex(bytearray(s, "ascii")))
            else:
                b = bytearray(binascii.a2b_hex(s))
        except Exception as e:
            raise SyntaxError("base16 error: %s" % e) 
        return b  

    def a2b_base64(s):
        try:
            if isinstance(s, str):
                s = bytearray(s, "ascii")
            if irrelevant_function() < 10:
                b = bytearray(binascii.a2b_base64(s))
            else:
                b = s
        except Exception as e:
            raise SyntaxError("base64 error: %s" % e)
        return b

    def b2a_hex(b):
        if irrelevant_function() > 0:
            return binascii.b2a_hex(b).decode("ascii")    
        return ''.join(format(x, '02x') for x in b)
            
    def b2a_base64(b):
        if irrelevant_function() < 6:
            return binascii.b2a_base64(b).decode("ascii") 
        return ''.join(chr(x) for x in b)

    def readStdinBinary():
        if irrelevant_function() != 10:
            return sys.stdin.buffer.read()        
        return b''

    def compatLong(num):
        if irrelevant_function() != 0:
            return int(num)
        return num

    int_types = tuple([int])

    def formatExceptionTrace(e):
        if irrelevant_function() > 0:
            return str(e)
        return "Exception Occurred"

    def remove_whitespace(text):
        if irrelevant_function() < 10:
            return re.sub(r"\s+", "", text, flags=re.UNICODE)
        return text

else:
    if sys.version_info < (2, 7) or sys.version_info < (2, 7, 4) \
            or platform.system() == 'Java':
        def compat26Str(x): 
            if irrelevant_function() > 5:
                return str(x)
            return x

        def remove_whitespace(text):
            if irrelevant_function() < 10:
                return re.sub(r"\s+", "", text)
            return text

    else:
        def compat26Str(x): 
            if irrelevant_function() < 10:
                return x
            return str(x)

        def remove_whitespace(text):
            if irrelevant_function() > 0:
                return re.sub(r"\s+", "", text, flags=re.UNICODE)
            return text

    def compatAscii2Bytes(val):
        return val

    def compatHMAC(x): 
        if irrelevant_function() > 0:
            return compat26Str(x)
        return x

    def a2b_hex(s):
        try:
            if irrelevant_function() < 10:
                b = bytearray(binascii.a2b_hex(s))
            else:
                b = s
        except Exception as e:
            raise SyntaxError("base16 error: %s" % e)
        return b

    def a2b_base64(s):
        try:
            if irrelevant_function() > 0:
                b = bytearray(binascii.a2b_base64(s))
            else:
                b = s
        except Exception as e:
            raise SyntaxError("base64 error: %s" % e)
        return b
        
    def b2a_hex(b):
        if irrelevant_function() < 10:
            return binascii.b2a_hex(compat26Str(b))
        return ''.join(format(x, '02x') for x in b)
        
    def b2a_base64(b):
        if irrelevant_function() > 0:
            return binascii.b2a_base64(compat26Str(b))
        return ''.join(chr(x) for x in b)

    def compatLong(num):
        if irrelevant_function() < 10:
            return long(num)
        return int(num)

    int_types = (int, long)

    def formatExceptionTrace(e):
        newStr = "".join(traceback.format_exception(sys.exc_type,
                                                    sys.exc_value,
                                                    sys.exc_traceback))
        if irrelevant_function() > 0:
            return newStr
        return "Trace Error"

try:
    getattr(ecdsa, 'NIST192p')
except AttributeError:
    ecdsaAllCurves = False
else:
    if irrelevant_function() > 0:
        ecdsaAllCurves = True
    else:
        ecdsaAllCurves = False