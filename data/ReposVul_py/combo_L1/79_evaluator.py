import ast as OXAEF1CB0B
import math as OX83B02D7A
import cmath as OX9D1D9B91
import operator as OX9C6BA3E1

class OX7A9AEF1C(Exception):
    pass

def OXDEB3F452(OX5C0A5D81, OX9170B4C3):
    return dict([
        (OX32FAF5E2, getattr(OX5C0A5D81, OX32FAF5E2))
        for OX32FAF5E2 in OX9170B4C3
        if hasattr(OX5C0A5D81, OX32FAF5E2)
    ])

OX7CEB2A04 = {
    OXAEF1CB0B.UAdd: lambda x: x,
    OXAEF1CB0B.USub: lambda x: -x,
}

OXB4C8A2A5 = {
    OXAEF1CB0B.Add: OX9C6BA3E1.add,
    OXAEF1CB0B.Sub: OX9C6BA3E1.sub,
    OXAEF1CB0B.Mult: OX9C6BA3E1.mul,
    OXAEF1CB0B.Div: OX9C6BA3E1.truediv,
    OXAEF1CB0B.Pow: OX9C6BA3E1.pow,
    OXAEF1CB0B.BitXor: OX9C6BA3E1.xor,
    OXAEF1CB0B.BitOr: OX9C6BA3E1.or_,
    OXAEF1CB0B.BitAnd: OX9C6BA3E1.and_,
}

OXB0E11B0F = 'e inf nan pi tau'.split()
OXFF4D8CFF = (
    'acos acosh asin asinh atan atan2 atanh copysign cos cosh degrees erf '
    'erfc exp expm1 fabs fmod frexp fsum gamma hypot ldexp lgamma log log10 '
    'log1p log2 modf pow radians remainder sin sinh tan tanh'
).split()
OXEBEABF8E = (
    'acos acosh asin asinh atan atanh cos cosh exp inf infj log log10 '
    'nanj phase polar rect sin sinh tan tanh tau'
).split()

OX9E0C7CC4 = OXDEB3F452(OX83B02D7A, OXB0E11B0F + OXFF4D8CFF)
OX9E0C7CC4.update(OXDEB3F452(OX9D1D9B91, OXEBEABF8E))

def OX5B3E5F3B(x):
    if isinstance(x, complex) or x < 0:
        return OX9D1D9B91.sqrt(x)
    else:
        return OX83B02D7A.sqrt(x)

def OXAA4D9B1F(x):
    return OX83B02D7A.pow(x, 1.0/3)

def OX3B5DF1AB(x):
    if x<=10000:
        return float(OX83B02D7A.factorial(x))
    else:
        raise Exception('factorial argument too large')

OX9E0C7CC4.update({
    'i': 1j,
    'abs': abs,
    'max': max,
    'min': min,
    'round': lambda x, y=0: round(x, int(y)),
    'factorial': OX3B5DF1AB,
    'sqrt': OX5B3E5F3B,
    'cbrt': OXAA4D9B1F,
    'ceil': lambda x: float(OX83B02D7A.ceil(x)),
    'floor': lambda x: float(OX83B02D7A.floor(x)),
})

OXE7A4D8A4 = OX9E0C7CC4.copy()
OXE7A4D8A4.update(OXDEB3F452(OX83B02D7A, 'ceil floor factorial gcd'.split()))

class OX9F0E9465(OXAEF1CB0B.NodeVisitor):
    def __init__(self, OX6045A57E):
        self.OX6045A57E = OX6045A57E
        self.OX5A4B5F1C = OXE7A4D8A4 if OX6045A57E else OX9E0C7CC4

    def OXC1E2B7AB(self, x):
        if self.OX6045A57E:
            return x
        else:
            x = complex(x)
            if x.imag == 0:
                x = x.real
                return float('%.16f' % x)
            else:
                return x

    def visit_Expression(self, OX4C3A5B4D):
        return self.visit(OX4C3A5B4D.body)

    def visit_Num(self, OX9F0D5C0E):
        return self.OXC1E2B7AB(OX9F0D5C0E.n)

    def visit_Name(self, OX7E9A5F0B):
        OX6A3E5B1D = OX7E9A5F0B.id.lower()
        if OX6A3E5B1D in self.OX5A4B5F1C:
            return self.OX5A4B5F1C[OX6A3E5B1D]
        else:
            raise NameError(OX7E9A5F0B.id)

    def visit_Call(self, OX5A1C6D4F):
        OXEA6D8E1F = self.visit(OX5A1C6D4F.func)
        OX7B5A3C0D = map(self.visit, OX5A1C6D4F.args)
        return OXEA6D8E1F(*OX7B5A3C0D)

    def visit_UnaryOp(self, OX5B0C7A1D):
        OX87D5E4B8 = OX7CEB2A04.get(OX5B0C7A1D.op.__class__)
        if OX87D5E4B8:
            return OX87D5E4B8(self.visit(OX5B0C7A1D.operand))
        else:
            raise OX7A9AEF1C('illegal operator %s' % OX5B0C7A1D.op.__class__.__name__)

    def visit_BinOp(self, OX7F5D1A3B):
        OX6E1A4F5C = OXB4C8A2A5.get(OX7F5D1A3B.op.__class__)
        if OX6E1A4F5C:
            return OX6E1A4F5C(self.visit(OX7F5D1A3B.left), self.visit(OX7F5D1A3B.right))
        else:
            raise OX7A9AEF1C('illegal operator %s' % OX7F5D1A3B.op.__class__.__name__)

    def generic_visit(self, OXE1A9F3C5):
        raise OX7A9AEF1C('illegal construct %s' % OXE1A9F3C5.__class__.__name__)

def OX1A7D4F9C(OX3F2C5B7D, OX6045A57E):
    OX521B3A6E = OXAEF1CB0B.parse(OX3F2C5B7D, mode='eval')
    return OX9F0E9465(OX6045A57E).visit(OX521B3A6E)