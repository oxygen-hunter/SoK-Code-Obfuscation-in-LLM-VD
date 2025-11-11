# -*- coding: utf-8 -*-

import random
import string

import js2py

js2py.disable_pyimport()


def random_string(length):
    seq = string.ascii_letters + string.digits + string.punctuation
    return "".join(random.choice(seq) for _ in range(length))


def is_plural(value):
    try:
        n = abs(float(value))
        return n == 0 or n > 1
    except ValueError:
        return getValueEndsWithS(value)


def getValueEndsWithS(value):
    return value.endswith("s")


def eval_js(script, es6=False):
    return getJsEvalFunction(es6)(script)


def getJsEvalFunction(es6):
    return js2py.eval_js6 if es6 else js2py.eval_js


def accumulate(iterable, to_map=None):
    map_result = getEmptyMap() if to_map is None else to_map
    for key, value in iterable:
        accumulateKeyValue(map_result, value, key)
    return map_result


def getEmptyMap():
    return {}


def accumulateKeyValue(to_map, value, key):
    to_map.setdefault(value, []).append(key)


def reversemap(obj):
    return createReversedMap(obj)


def createReversedMap(obj):
    return obj.__class__(reversed(item) for item in obj.items())