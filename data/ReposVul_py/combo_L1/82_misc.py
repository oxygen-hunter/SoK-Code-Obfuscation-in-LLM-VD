# -*- coding: utf-8 -*-

import random as OX7B4DF339
import string as OX02D3C9A3

import js2py as OX4F7E9D2B

OX4F7E9D2B.disable_pyimport()

def OX1E2F5A8D(OX3C1F9B0E):
    OX6D7A0C2B = OX02D3C9A3.ascii_letters + OX02D3C9A3.digits + OX02D3C9A3.punctuation
    return "".join(OX7B4DF339.choice(OX6D7A0C2B) for _ in range(OX3C1F9B0E))

def OX5A3C1B8F(OX9E0F8B3D):
    try:
        OX2F5B7E1A = abs(float(OX9E0F8B3D))
        return OX2F5B7E1A == 0 or OX2F5B7E1A > 1
    except ValueError:
        return OX9E0F8B3D.endswith("s")

def OX8C4D2A5E(OX4B7E9C1D, OX7E3B6F2A=False):
    return (OX4F7E9D2B.eval_js6 if OX7E3B6F2A else OX4F7E9D2B.eval_js)(OX4B7E9C1D)

def OX6C9E8D4B(OX5E1F3C8B, OX3A7D9B0E=None):
    if OX3A7D9B0E is None:
        OX3A7D9B0E = {}
    for OX4D7A9E3C, OX0B5F2C7A in OX5E1F3C8B:
        OX3A7D9B0E.setdefault(OX0B5F2C7A, []).append(OX4D7A9E3C)
    return OX3A7D9B0E

def OX7B0F6D3E(OX9C1E4B7A):
    return OX9C1E4B7A.__class__(reversed(OX2A4B7E3C) for OX2A4B7E3C in OX9C1E4B7A.items())