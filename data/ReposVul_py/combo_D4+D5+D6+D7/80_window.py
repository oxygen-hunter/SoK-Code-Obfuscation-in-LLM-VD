import math
from typing import List
from typing import Tuple
from typing import Union

import paddle
from paddle import Tensor

class WindowFunctionRegister(object):
    def __init__(self):
        self._dict = dict()

    def register(self, func=None):
        def add_subfunction(func):
            self._dict[func.__name__] = func
            return func

        return add_subfunction

    def get(self, name):
        return self._dict[name]

window_function_register = WindowFunctionRegister()

@window_function_register.register()
def _cat(x: List[Tensor], t: str) -> Tensor:
    l = [paddle.to_tensor(_, t) for _ in x]
    return paddle.concat(l)

@window_function_register.register()
def _acosh(x: Union[Tensor, float]) -> Tensor:
    if isinstance(x, float):
        return math.log(x + math.sqrt(x**2 - 1))
    return paddle.log(x + paddle.sqrt(paddle.square(x) - 1))

@window_function_register.register()
def _extend(M_sym: Tuple[int, bool]) -> Tuple[int, bool]:
    if not M_sym[1]:
        return M_sym[0] + 1, True
    else:
        return M_sym

@window_function_register.register()
def _len_guards(M: int) -> bool:
    if int(M) != M or M < 0:
        raise ValueError('Window length M must be a non-negative integer')

    return M <= 1

@window_function_register.register()
def _truncate(w: Tensor, needed: bool) -> Tensor:
    if needed:
        return w[:-1]
    else:
        return w

@window_function_register.register()
def _general_gaussian(
    M_sym: Tuple[int, bool], ps: Tuple[float, float], dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M_sym[0]):
        return paddle.ones((M_sym[0],), dtype=dtype)
    M_sym = _extend(M_sym)

    n = paddle.arange(0, M_sym[0], dtype=dtype) - (M_sym[0] - 1.0) / 2.0
    w = paddle.exp(-0.5 * paddle.abs(n / ps[1]) ** (2 * ps[0]))

    return _truncate(w, M_sym[1])

@window_function_register.register()
def _general_cosine(
    M_sym: Tuple[int, bool], a: List[float], dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M_sym[0]):
        return paddle.ones((M_sym[0],), dtype=dtype)
    M_sym = _extend(M_sym)
    fac = paddle.linspace(-math.pi, math.pi, M_sym[0], dtype=dtype)
    w = paddle.zeros((M_sym[0],), dtype=dtype)
    for k in range(len(a)):
        w += a[k] * paddle.cos(k * fac)
    return _truncate(w, M_sym[1])

@window_function_register.register()
def _general_hamming(
    M_sym: Tuple[int, bool], alpha: float, dtype: str = 'float64'
) -> Tensor:
    return _general_cosine(M_sym, [alpha, 1.0 - alpha], dtype=dtype)

@window_function_register.register()
def _taylor(
    M_sym: Tuple[int, bool], nbar_sll_norm: Tuple[int, float, bool], dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M_sym[0]):
        return paddle.ones((M_sym[0],), dtype=dtype)
    M_sym = _extend(M_sym)
    B = 10 ** (nbar_sll_norm[1] / 20)
    A = _acosh(B) / math.pi
    s2 = nbar_sll_norm[0]**2 / (A**2 + (nbar_sll_norm[0] - 0.5) ** 2)
    ma = paddle.arange(1, nbar_sll_norm[0], dtype=dtype)

    Fm = paddle.empty((nbar_sll_norm[0] - 1,), dtype=dtype)
    signs = paddle.empty_like(ma)
    signs[::2] = 1
    signs[1::2] = -1
    m2 = ma * ma
    for mi in range(len(ma)):
        numer = signs[mi] * paddle.prod(
            1 - m2[mi] / s2 / (A**2 + (ma - 0.5) ** 2)
        )
        if mi == 0:
            denom = 2 * paddle.prod(1 - m2[mi] / m2[mi + 1 :])
        elif mi == len(ma) - 1:
            denom = 2 * paddle.prod(1 - m2[mi] / m2[:mi])
        else:
            denom = (
                2
                * paddle.prod(1 - m2[mi] / m2[:mi])
                * paddle.prod(1 - m2[mi] / m2[mi + 1 :])
            )

        Fm[mi] = numer / denom

    def W(n):
        return 1 + 2 * paddle.matmul(
            Fm.unsqueeze(0),
            paddle.cos(2 * math.pi * ma.unsqueeze(1) * (n - M_sym[0] / 2.0 + 0.5) / M_sym[0]),
        )

    w = W(paddle.arange(0, M_sym[0], dtype=dtype))

    if nbar_sll_norm[2]:
        scale = 1.0 / W((M_sym[0] - 1) / 2)
        w *= scale
    w = w.squeeze()
    return _truncate(w, M_sym[1])

@window_function_register.register()
def _hamming(M_sym: Tuple[int, bool], dtype: str = 'float64') -> Tensor:
    return _general_hamming(M_sym, 0.54, dtype=dtype)

@window_function_register.register()
def _hann(M_sym: Tuple[int, bool], dtype: str = 'float64') -> Tensor:
    return _general_hamming(M_sym, 0.5, dtype=dtype)

@window_function_register.register()
def _tukey(
    M_sym: Tuple[int, bool], alpha: float, dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M_sym[0]):
        return paddle.ones((M_sym[0],), dtype=dtype)

    if alpha <= 0:
        return paddle.ones((M_sym[0],), dtype=dtype)
    elif alpha >= 1.0:
        return hann(M_sym[0], sym=M_sym[1])

    M_sym = _extend(M_sym)

    n = paddle.arange(0, M_sym[0], dtype=dtype)
    width = int(alpha * (M_sym[0] - 1) / 2.0)
    n1 = n[0 : width + 1]
    n2 = n[width + 1 : M_sym[0] - width - 1]
    n3 = n[M_sym[0] - width - 1 :]

    w1 = 0.5 * (1 + paddle.cos(math.pi * (-1 + 2.0 * n1 / alpha / (M_sym[0] - 1))))
    w2 = paddle.ones(n2.shape, dtype=dtype)
    w3 = 0.5 * (
        1
        + paddle.cos(math.pi * (-2.0 / alpha + 1 + 2.0 * n3 / alpha / (M_sym[0] - 1)))
    )
    w = paddle.concat([w1, w2, w3])

    return _truncate(w, M_sym[1])

@window_function_register.register()
def _kaiser(
    M_sym: Tuple[int, bool], beta: float, dtype: str = 'float64'
) -> Tensor:
    raise NotImplementedError()

@window_function_register.register()
def _gaussian(
    M_sym: Tuple[int, bool], std: float, dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M_sym[0]):
        return paddle.ones((M_sym[0],), dtype=dtype)
    M_sym = _extend(M_sym)

    n = paddle.arange(0, M_sym[0], dtype=dtype) - (M_sym[0] - 1.0) / 2.0
    sig2 = 2 * std * std
    w = paddle.exp(-(n**2) / sig2)

    return _truncate(w, M_sym[1])

@window_function_register.register()
def _exponential(
    M_sym: Tuple[int, bool], center_tau: Tuple[float, float], dtype: str = 'float64'
) -> Tensor:
    if M_sym[1] and center_tau[0] is not None:
        raise ValueError("If sym==True, center must be None.")
    if _len_guards(M_sym[0]):
        return paddle.ones((M_sym[0],), dtype=dtype)
    M_sym = _extend(M_sym)

    if center_tau[0] is None:
        center_tau = ((M_sym[0] - 1) / 2, center_tau[1])

    n = paddle.arange(0, M_sym[0], dtype=dtype)
    w = paddle.exp(-paddle.abs(n - center_tau[0]) / center_tau[1])

    return _truncate(w, M_sym[1])

@window_function_register.register()
def _triang(M_sym: Tuple[int, bool], dtype: str = 'float64') -> Tensor:
    if _len_guards(M_sym[0]):
        return paddle.ones((M_sym[0],), dtype=dtype)
    M_sym = _extend(M_sym)

    n = paddle.arange(1, (M_sym[0] + 1) // 2 + 1, dtype=dtype)
    if M_sym[0] % 2 == 0:
        w = (2 * n - 1.0) / M_sym[0]
        w = paddle.concat([w, w[::-1]])
    else:
        w = 2 * n / (M_sym[0] + 1.0)
        w = paddle.concat([w, w[-2::-1]])

    return _truncate(w, M_sym[1])

@window_function_register.register()
def _bohman(M_sym: Tuple[int, bool], dtype: str = 'float64') -> Tensor:
    if _len_guards(M_sym[0]):
        return paddle.ones((M_sym[0],), dtype=dtype)
    M_sym = _extend(M_sym)

    fac = paddle.abs(paddle.linspace(-1, 1, M_sym[0], dtype=dtype)[1:-1])
    w = (1 - fac) * paddle.cos(math.pi * fac) + 1.0 / math.pi * paddle.sin(
        math.pi * fac
    )
    w = _cat([0, w, 0], dtype)

    return _truncate(w, M_sym[1])

@window_function_register.register()
def _blackman(M_sym: Tuple[int, bool], dtype: str = 'float64') -> Tensor:
    return _general_cosine(M_sym, [0.42, 0.50, 0.08], dtype=dtype)

@window_function_register.register()
def _cosine(M_sym: Tuple[int, bool], dtype: str = 'float64') -> Tensor:
    if _len_guards(M_sym[0]):
        return paddle.ones((M_sym[0],), dtype=dtype)
    M_sym = _extend(M_sym)
    w = paddle.sin(math.pi / M_sym[0] * (paddle.arange(0, M_sym[0], dtype=dtype) + 0.5))

    return _truncate(w, M_sym[1])

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

    try:
        winfunc = window_function_register.get('_' + winstr)
    except KeyError as e:
        raise ValueError("Unknown window type.") from e

    M_sym = (win_length, sym)
    kwargs = {'dtype': dtype}
    if winstr in {'general_gaussian', 'general_cosine', 'general_hamming', 'taylor'}:
        if winstr == 'general_gaussian':
            ps = args
            return winfunc(M_sym, ps, **kwargs)
        elif winstr == 'general_cosine':
            a = args[0]
            return winfunc(M_sym, a, **kwargs)
        elif winstr == 'general_hamming':
            alpha = args[0]
            return winfunc(M_sym, alpha, **kwargs)
        elif winstr == 'taylor':
            nbar_sll_norm = args
            return winfunc(M_sym, nbar_sll_norm, **kwargs)
    elif winstr in {'tukey', 'gaussian', 'exponential'}:
        if winstr == 'tukey':
            alpha = args[0]
            return winfunc(M_sym, alpha, **kwargs)
        elif winstr == 'gaussian':
            std = args[0]
            return winfunc(M_sym, std, **kwargs)
        elif winstr == 'exponential':
            center_tau = args
            return winfunc(M_sym, center_tau, **kwargs)
    else:
        return winfunc(M_sym, **kwargs)