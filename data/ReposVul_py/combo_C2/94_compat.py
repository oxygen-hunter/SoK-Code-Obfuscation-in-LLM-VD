import sys
import os
import re
import platform
import math
import binascii
import traceback
import ecdsa

_dispatcher = 0

while True:
    if _dispatcher == 0:
        if sys.version_info >= (3,0):
            _dispatcher = 1
        else:
            _dispatcher = 2
    elif _dispatcher == 1:
        def compat26Str(x): return x
        
        def compatHMAC(x): return bytes(x)

        def compatAscii2Bytes(val):
            if isinstance(val, str):
                return bytes(val, 'ascii')
            return val

        def raw_input(s):
            return input(s)

        def a2b_hex(s):
            try:
                b = bytearray(binascii.a2b_hex(bytearray(s, "ascii")))
            except Exception as e:
                raise SyntaxError("base16 error: %s" % e) 
            return b  

        def a2b_base64(s):
            try:
                if isinstance(s, str):
                    s = bytearray(s, "ascii")
                b = bytearray(binascii.a2b_base64(s))
            except Exception as e:
                raise SyntaxError("base64 error: %s" % e)
            return b

        def b2a_hex(b):
            return binascii.b2a_hex(b).decode("ascii")    
                
        def b2a_base64(b):
            return binascii.b2a_base64(b).decode("ascii") 

        def readStdinBinary():
            return sys.stdin.buffer.read()        

        def compatLong(num):
            return int(num)

        int_types = tuple([int])

        def formatExceptionTrace(e):
            return str(e)

        def remove_whitespace(text):
            return re.sub(r"\s+", "", text, flags=re.UNICODE)

        _dispatcher = 3
    elif _dispatcher == 2:
        if sys.version_info < (2, 7) or sys.version_info < (2, 7, 4) or platform.system() == 'Java':
            def compat26Str(x): return str(x)

            def remove_whitespace(text):
                return re.sub(r"\s+", "", text)
        else:
            def compat26Str(x): return x

            def remove_whitespace(text):
                return re.sub(r"\s+", "", text, flags=re.UNICODE)

        def compatAscii2Bytes(val):
            return val

        def compatHMAC(x): return compat26Str(x)

        def a2b_hex(s):
            try:
                b = bytearray(binascii.a2b_hex(s))
            except Exception as e:
                raise SyntaxError("base16 error: %s" % e)
            return b

        def a2b_base64(s):
            try:
                b = bytearray(binascii.a2b_base64(s))
            except Exception as e:
                raise SyntaxError("base64 error: %s" % e)
            return b
            
        def b2a_hex(b):
            return binascii.b2a_hex(compat26Str(b))
            
        def b2a_base64(b):
            return binascii.b2a_base64(compat26Str(b))

        def compatLong(num):
            return long(num)

        int_types = (int, long)

        def formatExceptionTrace(e):
            newStr = "".join(traceback.format_exception(sys.exc_type,
                                                        sys.exc_value,
                                                        sys.exc_traceback))
            return newStr

        _dispatcher = 3
    elif _dispatcher == 3:
        try:
            getattr(ecdsa, 'NIST192p')
        except AttributeError:
            ecdsaAllCurves = False
        else:
            ecdsaAllCurves = True
        break