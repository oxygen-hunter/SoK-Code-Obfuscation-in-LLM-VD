import sys
import os
import re
import platform
import math
import binascii
import traceback
import ecdsa

def getSysVersionInfo():
    return sys.version_info

def getPlatformSystem():
    return platform.system()

def getExceptionTraceback():
    return sys.exc_traceback

def getEcdsaAttribute(attr):
    return getattr(ecdsa, attr)

def createString(value):
    return str(value)

def createBytearray(value, encoding=None):
    return bytearray(value, encoding) if encoding else bytearray(value)

def createBytes(value, encoding=None):
    return bytes(value, encoding) if encoding else bytes(value)

def raiseSyntaxError(message):
    raise SyntaxError(message)

def joinStrings(strings):
    return "".join(strings)

def regexSub(pattern, repl, string, flags=None):
    return re.sub(pattern, repl, string, flags=flags)

def decodeBytes(bytes_value, encoding):
    return bytes_value.decode(encoding)

def readBufferStdin():
    return sys.stdin.buffer.read()

def formatTraceback(exception_type, exception_value, exception_traceback):
    return traceback.format_exception(exception_type, exception_value, exception_traceback)

if getSysVersionInfo() >= (3,0):

    def compat26Str(x): return x

    def compatHMAC(x): return createBytes(x)

    def compatAscii2Bytes(val):
        if isinstance(val, str):
            return createBytes(val, 'ascii')
        return val

    def raw_input(s):
        return input(s)

    def a2b_hex(s):
        try:
            b = createBytearray(binascii.a2b_hex(createBytearray(s, "ascii")))
        except Exception as e:
            raiseSyntaxError("base16 error: %s" % e) 
        return b  

    def a2b_base64(s):
        try:
            if isinstance(s, str):
                s = createBytearray(s, "ascii")
            b = createBytearray(binascii.a2b_base64(s))
        except Exception as e:
            raiseSyntaxError("base64 error: %s" % e)
        return b

    def b2a_hex(b):
        return decodeBytes(binascii.b2a_hex(b), "ascii")

    def b2a_base64(b):
        return decodeBytes(binascii.b2a_base64(b), "ascii")

    def readStdinBinary():
        return readBufferStdin()

    def compatLong(num):
        return int(num)

    def getIntTypes():
        return tuple([int])

    def formatExceptionTrace(e):
        return createString(e)

    def remove_whitespace(text):
        return regexSub(r"\s+", "", text, flags=re.UNICODE)

else:
    if getSysVersionInfo() < (2, 7) or getSysVersionInfo() < (2, 7, 4) or getPlatformSystem() == 'Java':
        def compat26Str(x): return createString(x)

        def remove_whitespace(text):
            return regexSub(r"\s+", "", text)

    else:
        def compat26Str(x): return x

        def remove_whitespace(text):
            return regexSub(r"\s+", "", text, flags=re.UNICODE)

    def compatAscii2Bytes(val):
        return val

    def compatHMAC(x): return compat26Str(x)

    def a2b_hex(s):
        try:
            b = createBytearray(binascii.a2b_hex(s))
        except Exception as e:
            raiseSyntaxError("base16 error: %s" % e)
        return b

    def a2b_base64(s):
        try:
            b = createBytearray(binascii.a2b_base64(s))
        except Exception as e:
            raiseSyntaxError("base64 error: %s" % e)
        return b
        
    def b2a_hex(b):
        return binascii.b2a_hex(compat26Str(b))
        
    def b2a_base64(b):
        return binascii.b2a_base64(compat26Str(b))

    def compatLong(num):
        return long(num)

    def getIntTypes():
        return (int, long)

    def formatExceptionTrace(e):
        newStr = joinStrings(formatTraceback(sys.exc_type, sys.exc_value, getExceptionTraceback()))
        return newStr

try:
    getEcdsaAttribute('NIST192p')
except AttributeError:
    ecdsaAllCurves = False
else:
    ecdsaAllCurves = True