import sys, os, re, platform, math, binascii, traceback, ecdsa

def compat26Str(x): return x if sys.version_info >= (3,0) else str(x)

def compatHMAC(x): return bytes(x) if sys.version_info >= (3,0) else compat26Str(x)

def compatAscii2Bytes(val):
    if sys.version_info >= (3,0):
        if isinstance(val, str):
            return bytes(val, 'ascii')
    return val

def raw_input(s): return input(s) if sys.version_info >= (3,0) else raw_input(s)

def a2b_hex(s):
    try:
        b = bytearray(binascii.a2b_hex(bytearray(s, "ascii"))) if sys.version_info >= (3,0) else bytearray(binascii.a2b_hex(s))
    except Exception as e:
        raise SyntaxError(f"base16 error: {e}")
    return b

def a2b_base64(s):
    try:
        if sys.version_info >= (3,0) and isinstance(s, str):
            s = bytearray(s, "ascii")
        b = bytearray(binascii.a2b_base64(s))
    except Exception as e:
        raise SyntaxError(f"base64 error: {e}")
    return b

def b2a_hex(b): return binascii.b2a_hex(b).decode("ascii") if sys.version_info >= (3,0) else binascii.b2a_hex(compat26Str(b))

def b2a_base64(b): return binascii.b2a_base64(b).decode("ascii") if sys.version_info >= (3,0) else binascii.b2a_base64(compat26Str(b))

def readStdinBinary(): return sys.stdin.buffer.read() if sys.version_info >= (3,0) else sys.stdin.read()

def compatLong(num): return int(num) if sys.version_info >= (3,0) else long(num)

int_types = tuple([int]) if sys.version_info >= (3,0) else (int, long)

def formatExceptionTrace(e): return str(e) if sys.version_info >= (3,0) else "".join(traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_traceback))

def remove_whitespace(text): return re.sub(r"\s+", "", text, flags=re.UNICODE) if sys.version_info >= (3,0) or sys.version_info >= (2, 7) and sys.version_info >= (2, 7, 4) and platform.system() != 'Java' else re.sub(r"\s+", "", text)

try: getattr(ecdsa, 'NIST192p')
except AttributeError: ecdsaAllCurves = False
else: ecdsaAllCurves = True