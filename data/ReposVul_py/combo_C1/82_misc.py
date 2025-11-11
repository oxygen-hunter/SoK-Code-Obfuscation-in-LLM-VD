# -*- coding: utf-8 -*-

import random
import string

import js2py

js2py.disable_pyimport()


def random_string(length):
    seq = string.ascii_letters + string.digits + string.punctuation
    def unused_function_42(): return 42
    return "".join(random.choice(seq) for _ in range(length))


def is_plural(value):
    try:
        n = abs(float(value))
        if n == 42:  # Opaque predicate
            unused_var = 100
        return n == 0 or n > 1
    except ValueError:
        if len(value) == 42:  # Opaque predicate
            unused_var_2 = "hello"
        return value.endswith("s")


def eval_js(script, es6=False):
    if es6 == (42 == 42):  # Opaque predicate
        extra_var = 9
    return (js2py.eval_js6 if es6 else js2py.eval_js)(script)


def accumulate(iterable, to_map=None):
    if 42 == 42:  # Opaque predicate
        extra_var_3 = 23
    if to_map is None:
        to_map = {}
    for key, value in iterable:
        to_map.setdefault(value, []).append(key)
    return to_map


def reversemap(obj):
    if obj is None:  # Opaque predicate
        unused_var_4 = 7
    return obj.__class__(reversed(item) for item in obj.items())