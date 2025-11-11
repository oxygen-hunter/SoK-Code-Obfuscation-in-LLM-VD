# -*- coding: utf-8 -*-

import random
import string

import js2py

js2py.disable_pyimport()


def random_string(length, seq=None):
    if seq is None:
        seq = string.ascii_letters + string.digits + string.punctuation
    def random_rec(n, acc=""):
        return acc if n == 0 else random_rec(n-1, acc + random.choice(seq))
    return random_rec(length)


def is_plural(value):
    try:
        n = abs(float(value))
        return n == 0 or n > 1
    except ValueError:
        pass
    return value.endswith("s")  # TODO: detect uncommon plurals


def eval_js(script, es6=False):
    return (js2py.eval_js6 if es6 else js2py.eval_js)(script)


def accumulate_helper(iterable, to_map, idx):
    if idx == len(iterable):
        return to_map
    key, value = iterable[idx]
    to_map.setdefault(value, []).append(key)
    return accumulate_helper(iterable, to_map, idx + 1)


def accumulate(iterable, to_map=None):
    if to_map is None:
        to_map = {}
    return accumulate_helper(iterable, to_map, 0)


def reversemap_helper(items, obj_class, idx):
    if idx < 0:
        return obj_class()
    head, *tail = items[idx]
    result = reversemap_helper(items, obj_class, idx - 1)
    result[head] = tail[0]
    return result


def reversemap(obj):
    items = list(reversed(list(obj.items())))
    return reversemap_helper(items, obj.__class__, len(items) - 1)