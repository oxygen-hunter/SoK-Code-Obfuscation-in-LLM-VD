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

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self, instructions):
        self.instructions = instructions
        self.pc = 0
        while self.pc < len(self.instructions):
            op, *args = self.instructions[self.pc]
            getattr(self, f'op_{op}')(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a - b)

    def op_MUL(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a * b)

    def op_DIV(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a / b)

    def op_CALL(self, func_name, num_args):
        args = [self.op_POP() for _ in range(num_args)][::-1]
        result = SAFE_ENV[func_name](*args)
        self.op_PUSH(result)

    def op_JMP(self, target):
        self.pc = target - 1

    def op_JZ(self, target):
        if not self.op_POP():
            self.pc = target - 1

UNARY_OPS = {
    ast.UAdd: 'NOP',
    ast.USub: 'NEG',
}

BIN_OPS = {
    ast.Add: 'ADD',
    ast.Sub: 'SUB',
    ast.Mult: 'MUL',
    ast.Div: 'DIV',
    ast.Pow: 'CALL',
    ast.BitXor: 'CALL',
    ast.BitOr: 'CALL',
    ast.BitAnd: 'CALL',
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
        self.instructions = []

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
        value = self._convert_num(node.n)
        self.instructions.append(('PUSH', value))
        return value

    def visit_Name(self, node):
        id_ = node.id.lower()
        if id_ in self._env:
            self.instructions.append(('PUSH', self._env[id_]))
            return self._env[id_]
        else:
            raise NameError(node.id)

    def visit_Call(self, node):
        func = node.func.id
        for arg in node.args:
            self.visit(arg)
        self.instructions.append(('CALL', func, len(node.args)))

    def visit_UnaryOp(self, node):
        op = UNARY_OPS.get(node.op.__class__)
        if op == 'NEG':
            self.visit(node.operand)
            self.instructions.append(('PUSH', -1))
            self.instructions.append(('MUL',))
        elif op == 'NOP':
            self.visit(node.operand)
        else:
            raise InvalidNode('illegal operator %s' % node.op.__class__.__name__)

    def visit_BinOp(self, node):
        op = BIN_OPS.get(node.op.__class__)
        if op:
            self.visit(node.left)
            self.visit(node.right)
            if op == 'CALL':
                self.instructions.append(('CALL', node.op.__class__.__name__.lower(), 2))
            else:
                self.instructions.append((op,))
        else:
            raise InvalidNode('illegal operator %s' % node.op.__class__.__name__)

    def generic_visit(self, node):
        raise InvalidNode('illegal construct %s' % node.__class__.__name__)

def safe_eval(text, allow_ints):
    node = ast.parse(text, mode='eval')
    visitor = SafeEvalVisitor(allow_ints)
    result = visitor.visit(node)
    vm = VM()
    vm.run(visitor.instructions)
    return vm.op_POP()