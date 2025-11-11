import ast
import math
import cmath
import operator

class InvalidNode(Exception):
    pass

def filter_module(module, safe_names):
    return dict([
        (name, getattr(module, name))
        for name in safe_names
        if hasattr(module, name)
    ])

UNARY_OPS = {
    ast.UAdd: lambda x: x,
    ast.USub: lambda x: -x,
}

_0, _1, _2, _3, _4, _5, _6, _7, _8 = (
    ast.Add, 
    ast.Sub, 
    ast.Mult, 
    ast.Div, 
    ast.Pow, 
    ast.BitXor, 
    ast.BitOr, 
    ast.BitAnd, 
    operator.add
)

_9 = {
    _0: _8,
    _1: operator.sub,
    _2: operator.mul,
    _3: operator.truediv,
    _4: operator.pow,
    _5: operator.xor,
    _6: operator.or_,
    _7: operator.and_,
}

MATH_CONSTANTS = 'e inf nan pi tau'.split()
SAFE_MATH_FUNCTIONS = (
    'acos acosh asin asinh atan atan2 atanh copysign cos cosh degrees erf '
    'erfc exp expm1 fabs fmod frexp fsum gamma hypot ldexp lgamma log log10 '
    'log1p log2 modf pow radians remainder sin sinh tan tanh'
).split()
SAFE_CMATH_FUNCTIONS = (
    'acos acosh asin asinh atan atanh cos cosh exp inf infj log log10 '
    'nanj phase polar rect sin sinh tan tanh tau'
).split()

SAFE_ENV = filter_module(math, MATH_CONSTANTS + SAFE_MATH_FUNCTIONS)
SAFE_ENV.update(filter_module(cmath, SAFE_CMATH_FUNCTIONS))

def _sqrt(x):
    if isinstance(x, complex) or x < 0:
        return cmath.sqrt(x)
    else:
        return math.sqrt(x)

def _cbrt(x):
    return math.pow(x, 1.0/3)

def _factorial(x):
    if x<=10000:
        return float(math.factorial(x))
    else:
        raise Exception('factorial argument too large')

SAFE_ENV.update({
    'i': 1j,
    'abs': abs,
    'max': max,
    'min': min,
    'round': lambda x, y=0: round(x, int(y)),
    'factorial': _factorial,
    'sqrt': _sqrt,
    'cbrt': _cbrt,
    'ceil': lambda x: float(math.ceil(x)),
    'floor': lambda x: float(math.floor(x)),
})

UNSAFE_ENV = SAFE_ENV.copy()
UNSAFE_ENV.update(filter_module(math, 'ceil floor factorial gcd'.split()))

class SafeEvalVisitor(ast.NodeVisitor):
    def __init__(self, allow_ints):
        self._allow_ints = allow_ints
        self._env = UNSAFE_ENV if allow_ints else SAFE_ENV

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
        op = UNARY_OPS.get(node.op.__class__)
        if op:
            return op(self.visit(node.operand))
        else:
            raise InvalidNode('illegal operator %s' % node.op.__class__.__name__)

    def visit_BinOp(self, node):
        op = _9.get(node.op.__class__)
        if op:
            return op(self.visit(node.left), self.visit(node.right))
        else:
            raise InvalidNode('illegal operator %s' % node.op.__class__.__name__)

    def generic_visit(self, node):
        raise InvalidNode('illegal construct %s' % node.__class__.__name__)

def safe_eval(text, allow_ints):
    node = ast.parse(text, mode='eval')
    return SafeEvalVisitor(allow_ints).visit(node)