import ast as _ast
import ctypes
import sys

class InvalidNode(Exception):
    pass

def _filter_module(module, safe_names):
    return dict([
        (name, getattr(module, name))
        for name in safe_names
        if hasattr(module, name)
    ])

_UNARY_OPS = {
    _ast.UAdd: lambda x: x,
    _ast.USub: lambda x: -x,
}

_BIN_OPS = {
    _ast.Add: lambda x, y: x + y,
    _ast.Sub: lambda x, y: x - y,
    _ast.Mult: lambda x, y: x * y,
    _ast.Div: lambda x, y: x / y,
    _ast.Pow: lambda x, y: x ** y,
    _ast.BitXor: lambda x, y: int(x) ^ int(y),
    _ast.BitOr: lambda x, y: int(x) | int(y),
    _ast.BitAnd: lambda x, y: int(x) & int(y),
}

def _load_c_lib():
    if sys.platform.startswith('win'):
        return ctypes.CDLL('msvcrt.dll')
    elif sys.platform.startswith('darwin'):
        return ctypes.CDLL('libSystem.dylib')
    else:
        return ctypes.CDLL('libc.so.6')

_c_lib = _load_c_lib()

def _sqrt(x):
    if isinstance(x, complex) or x < 0:
        return complex(_c_lib.sqrt(ctypes.c_double(x.real)), _c_lib.sqrt(ctypes.c_double(x.imag)))
    else:
        return _c_lib.sqrt(ctypes.c_double(x))

def _cbrt(x):
    return x ** (1.0/3)

def _factorial(x):
    if x <= 10000:
        return float(_c_lib.tgamma(ctypes.c_double(x + 1)))
    else:
        raise Exception('factorial argument too large')

_SAFE_ENV = _filter_module(__import__('math'), 'e inf nan pi tau'.split() + 
    'acos acosh asin asinh atan atan2 atanh copysign cos cosh degrees erf '
    'erfc exp expm1 fabs fmod frexp fsum gamma hypot ldexp lgamma log log10 '
    'log1p log2 modf pow radians remainder sin sinh tan tanh'.split())
_SAFE_ENV.update(_filter_module(__import__('cmath'), 
    'acos acosh asin asinh atan atanh cos cosh exp inf infj log log10 '
    'nanj phase polar rect sin sinh tan tanh tau'.split()))

_SAFE_ENV.update({
    'i': 1j,
    'abs': abs,
    'max': max,
    'min': min,
    'round': lambda x, y=0: round(x, int(y)),
    'factorial': _factorial,
    'sqrt': _sqrt,
    'cbrt': _cbrt,
    'ceil': lambda x: float(_c_lib.ceil(ctypes.c_double(x))),
    'floor': lambda x: float(_c_lib.floor(ctypes.c_double(x))),
})

_UNSAFE_ENV = _SAFE_ENV.copy()
_UNSAFE_ENV.update(_filter_module(__import__('math'), 'ceil floor factorial gcd'.split()))

class _SafeEvalVisitor(_ast.NodeVisitor):
    def __init__(self, allow_ints):
        self._allow_ints = allow_ints
        self._env = _UNSAFE_ENV if allow_ints else _SAFE_ENV

    def _convert_num(self, x):
        if self._allow_ints:
            return x
        else:
            x = complex(x)
            if x.imag == 0:
                x = x.real
                return float('%.16f' % x)
            else:
                return x

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Num(self, node):
        return self._convert_num(node.n)

    def visit_Name(self, node):
        id_ = node.id.lower()
        if id_ in self._env:
            return self._env[id_]
        else:
            raise NameError(node.id)

    def visit_Call(self, node):
        func = self.visit(node.func)
        args = map(self.visit, node.args)
        return func(*args)

    def visit_UnaryOp(self, node):
        op = _UNARY_OPS.get(node.op.__class__)
        if op:
            return op(self.visit(node.operand))
        else:
            raise InvalidNode('illegal operator %s' % node.op.__class__.__name__)

    def visit_BinOp(self, node):
        op = _BIN_OPS.get(node.op.__class__)
        if op:
            return op(self.visit(node.left), self.visit(node.right))
        else:
            raise InvalidNode('illegal operator %s' % node.op.__class__.__name__)

    def generic_visit(self, node):
        raise InvalidNode('illegal construct %s' % node.__class__.__name__)

def safe_eval(text, allow_ints):
    node = _ast.parse(text, mode='eval')
    return _SafeEvalVisitor(allow_ints).visit(node)