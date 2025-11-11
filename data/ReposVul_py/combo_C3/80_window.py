import math
from typing import List, Tuple, Union
import paddle
from paddle import Tensor

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []
        self.functions = {}

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.pc < len(self.program):
            instr = self.program[self.pc]
            self.pc += 1
            self.execute(instr)

    def execute(self, instr):
        if instr[0] == 'PUSH':
            self.stack.append(instr[1])
        elif instr[0] == 'POP':
            self.stack.pop()
        elif instr[0] == 'ADD':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a + b)
        elif instr[0] == 'SUB':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(b - a)
        elif instr[0] == 'MUL':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a * b)
        elif instr[0] == 'DIV':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(b / a)
        elif instr[0] == 'JMP':
            self.pc = instr[1]
        elif instr[0] == 'JZ':
            if self.stack.pop() == 0:
                self.pc = instr[1]
        elif instr[0] == 'LOAD_FUNC':
            func_name = instr[1]
            self.stack.append(self.functions[func_name])
        elif instr[0] == 'CALL':
            func = self.stack.pop()
            args = [self.stack.pop() for _ in range(instr[1])]
            self.stack.append(func(*args))
        elif instr[0] == 'RETURN':
            pass

    def register_function(self, name, func):
        self.functions[name] = func

def _cat(x: List[Tensor], data_type: str) -> Tensor:
    l = [paddle.to_tensor(_, data_type) for _ in x]
    return paddle.concat(l)

def _acosh(x: Union[Tensor, float]) -> Tensor:
    if isinstance(x, float):
        return math.log(x + math.sqrt(x**2 - 1))
    return paddle.log(x + paddle.sqrt(paddle.square(x) - 1))

def _extend(M: int, sym: bool) -> bool:
    return M + 1 if not sym else M, not sym

def _len_guards(M: int) -> bool:
    if int(M) != M or M < 0:
        raise ValueError('Window length M must be a non-negative integer')
    return M <= 1

def _truncate(w: Tensor, needed: bool) -> Tensor:
    return w[:-1] if needed else w

def _general_gaussian(
    M: int, p, sig, sym: bool = True, dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)
    n = paddle.arange(0, M, dtype=dtype) - (M - 1.0) / 2.0
    w = paddle.exp(-0.5 * paddle.abs(n / sig) ** (2 * p))
    return _truncate(w, needs_trunc)

def _general_cosine(
    M: int, a: float, sym: bool = True, dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)
    fac = paddle.linspace(-math.pi, math.pi, M, dtype=dtype)
    w = paddle.zeros((M,), dtype=dtype)
    for k in range(len(a)):
        w += a[k] * paddle.cos(k * fac)
    return _truncate(w, needs_trunc)

def get_window(
    window: Union[str, Tuple[str, float]],
    win_length: int,
    fftbins: bool = True,
    dtype: str = 'float64',
) -> Tensor:
    sym = not fftbins
    args = ()
    if isinstance(window, tuple):
        winstr = window[0]
        if len(window) > 1:
            args = window[1:]
    elif isinstance(window, str):
        if window in ['gaussian', 'exponential']:
            raise ValueError(
                "The '" + window + "' window needs one or "
                "more parameters -- pass a tuple."
            )
        else:
            winstr = window
    else:
        raise ValueError(
            "%s as window type is not supported." % str(type(window))
        )

    vm = VM()
    vm.register_function('_general_gaussian', _general_gaussian)
    vm.register_function('_general_cosine', _general_cosine)
    program = [
        ('PUSH', win_length),
        ('PUSH', sym),
        ('LOAD_FUNC', '_' + winstr),
        ('CALL', 2),
        ('RETURN',)
    ]
    vm.load_program(program)
    vm.run()
    return vm.stack.pop()