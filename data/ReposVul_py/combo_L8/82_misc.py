# -*- coding: utf-8 -*-
import random
import string
from ctypes import CDLL, c_char_p, c_int

import js2py
js2py.disable_pyimport()

libc = CDLL("libc.so.6")
libc.srand(random.randint(0, 2**32 - 1))

def random_string(length):
    seq = (string.ascii_letters + string.digits + string.punctuation).encode('utf-8')
    buffer = (c_char_p * length)()
    for i in range(length):
        buffer[i] = c_char_p(random.choice(seq))
    return b"".join(buffer).decode('utf-8')

def is_plural(value):
    try:
        n = abs(float(value))
        return n == 0 or n > 1
    except ValueError:
        return value.endswith("s")

def eval_js(script, es6=False):
    return (js2py.eval_js6 if es6 else js2py.eval_js)(script)

def accumulate(iterable, to_map=None):
    if to_map is None:
        to_map = {}
    for key, value in iterable:
        to_map.setdefault(value, []).append(key)
    return to_map

def reversemap(obj):
    return obj.__class__(reversed(item) for item in obj.items())