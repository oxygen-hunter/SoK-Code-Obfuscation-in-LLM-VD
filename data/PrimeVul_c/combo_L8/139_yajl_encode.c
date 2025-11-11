import ctypes
import os

lib = ctypes.CDLL(os.path.join(os.path.dirname(__file__), 'yajl_encode.so'))

lib.CharToHex.argtypes = [ctypes.c_ubyte, ctypes.c_char_p]
lib.Utf32toUtf8.argtypes = [ctypes.c_uint, ctypes.c_char_p]
lib.hexToDigit.argtypes = [ctypes.POINTER(ctypes.c_uint), ctypes.c_char_p]

def yajl_string_encode(buf, str, htmlSafe):
    yajl_string_encode2(lib.yajl_buf_append, buf, str, htmlSafe)

def yajl_string_encode2(print_func, ctx, str, htmlSafe):
    beg = 0
    end = 0
    hexBuf = ctypes.create_string_buffer(7)
    hexBuf.value = b'\\u00'

    while end < len(str):
        escaped = None
        if str[end] in b'\r\n\\"/\f\b\t':
            escaped = {
                b'\r': b"\\r",
                b'\n': b"\\n",
                b'\\': b"\\\\",
                b'"': b"\\\"",
                b'\f': b"\\f",
                b'\b': b"\\b",
                b'\t': b"\\t",
                b'/': b"\\/" if htmlSafe else None
            }.get(str[end])
        elif str[end] < 32:
            lib.CharToHex(str[end], ctypes.byref(hexBuf, 4))
            escaped = hexBuf.value

        if escaped is not None:
            print_func(ctx, (ctypes.c_char * (end - beg)).from_buffer_copy(str[beg:end]), end - beg)
            print_func(ctx, escaped, len(escaped))
            beg = end = end + 1
        else:
            end += 1

    print_func(ctx, (ctypes.c_char * (end - beg)).from_buffer_copy(str[beg:end]), end - beg)

def yajl_string_decode(buf, str):
    beg = 0
    end = 0

    while end < len(str):
        if str[end:end+1] == b'\\':
            utf8Buf = ctypes.create_string_buffer(5)
            unescaped = b"?"
            lib.yajl_buf_append(buf, (ctypes.c_char * (end - beg)).from_buffer_copy(str[beg:end]), end - beg)

            end += 1
            if str[end:end+1] in b'rn\\/"fbt':
                unescaped = {
                    b'r': b"\r",
                    b'n': b"\n",
                    b'\\': b"\\",
                    b'/': b"/",
                    b'"': b"\"",
                    b'f': b"\f",
                    b'b': b"\b",
                    b't': b"\t"
                }[str[end:end+1]]
            elif str[end:end+1] == b'u':
                codepoint = ctypes.c_uint(0)
                lib.hexToDigit(ctypes.byref(codepoint), ctypes.c_char_p(str[end+1:end+5]))
                end += 4

                if (codepoint.value & 0xFC00) == 0xD800:
                    end += 1
                    if str[end:end+2] == b'\\u':
                        surrogate = ctypes.c_uint(0)
                        lib.hexToDigit(ctypes.byref(surrogate), ctypes.c_char_p(str[end+2:end+6]))
                        codepoint.value = (((codepoint.value & 0x3F) << 10) |
                                           (((codepoint.value >> 6) & 0xF) + 1) << 16 |
                                           (surrogate.value & 0x3FF))
                        end += 5
                    else:
                        unescaped = b"?"
                lib.Utf32toUtf8(codepoint.value, utf8Buf)
                unescaped = utf8Buf.value

                if codepoint.value == 0:
                    lib.yajl_buf_append(buf, unescaped, 1)
                    beg = end + 1
                    continue
            end += 1
            lib.yajl_buf_append(buf, unescaped, len(unescaped))
            beg = end
        else:
            end += 1
    lib.yajl_buf_append(buf, (ctypes.c_char * (end - beg)).from_buffer_copy(str[beg:end]), end - beg)