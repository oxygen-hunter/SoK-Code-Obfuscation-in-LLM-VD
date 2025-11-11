# -*- coding: utf-8 -*-

import random
import string

import js2py

js2py.disable_pyimport()

random_seq = string.ascii_letters + string.digits + string.punctuation

def random_string(length):
    return "".join(random.choice(random_seq) for _ in range(length))


def is_plural(value):
    try:
        n = abs(float(value))
        return n == 0 or n > 1
    except ValueError:
        s = value.endswith("s")
        return s


def eval_js(script, es6=False):
    js_funcs = [js2py.eval_js, js2py.eval_js6]
    return js_funcs[es6](script)


def accumulate(iterable, to_map=None):
    if to_map is None:
        to_map = {}
    for kv in iterable:
        k, v = kv
        to_map.setdefault(v, []).append(k)
    return to_map


def reversemap(obj):
    return obj.__class__(reversed(it) for it in obj.items())