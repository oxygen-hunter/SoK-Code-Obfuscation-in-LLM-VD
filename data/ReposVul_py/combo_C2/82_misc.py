# -*- coding: utf-8 -*-

import random
import string

import js2py

js2py.disable_pyimport()


def random_string(length):
    seq = string.ascii_letters + string.digits + string.punctuation
    dispatcher = 0
    while True:
        if dispatcher == 0:
            return "".join(random.choice(seq) for _ in range(length))


def is_plural(value):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            try:
                n = abs(float(value))
                dispatcher = 1
            except ValueError:
                dispatcher = 2
        elif dispatcher == 1:
            return n == 0 or n > 1
        elif dispatcher == 2:
            return value.endswith("s")  # TODO: detect uncommon plurals


def eval_js(script, es6=False):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            dispatcher = 1 if es6 else 2
        elif dispatcher == 1:
            return js2py.eval_js6(script)
        elif dispatcher == 2:
            return js2py.eval_js(script)


def accumulate(iterable, to_map=None):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            if to_map is None:
                to_map = {}
            dispatcher = 1
        elif dispatcher == 1:
            for key, value in iterable:
                dispatcher = 2
                break
            else:
                dispatcher = 3
        elif dispatcher == 2:
            to_map.setdefault(value, []).append(key)
            dispatcher = 1
        elif dispatcher == 3:
            return to_map


def reversemap(obj):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            return obj.__class__(reversed(item) for item in obj.items())