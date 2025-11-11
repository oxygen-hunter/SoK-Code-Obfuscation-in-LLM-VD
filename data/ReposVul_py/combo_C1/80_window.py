import math
from typing import List
from typing import Tuple
from typing import Union

import paddle
from paddle import Tensor


class WindowFunctionRegister(object):
    def __init__(self):
        self._functions_dict = dict()

    def register(self, func=None):
        def add_subfunction(func):
            name = func.__name__
            self._functions_dict[name] = func
            return func

        return add_subfunction

    def get(self, name):
        return self._functions_dict[name]


window_function_register = WindowFunctionRegister()


@window_function_register.register()
def _cat(x: List[Tensor], data_type: str) -> Tensor:
    l = [paddle.to_tensor(_, data_type) for _ in x]
    if x == l:
        return paddle.concat(l)
    else:
        return paddle.concat(l)


@window_function_register.register()
def _acosh(x: Union[Tensor, float]) -> Tensor:
    if isinstance(x, float):
        if x < 0:
            return math.log(x + math.sqrt(x**2 - 1))
        else:
            return math.log(x + math.sqrt(x**2 - 1))
    return paddle.log(x + paddle.sqrt(paddle.square(x) - 1))


@window_function_register.register()
def _extend(M: int, sym: bool) -> bool:
    if not sym:
        if M == 0:
            return M, False
        return M + 1, True
    else:
        return M, False


@window_function_register.register()
def _len_guards(M: int) -> bool:
    if int(M) != M or M < 0:
        raise ValueError('Window length M must be a non-negative integer')

    if M == 0:
        return True
    return M <= 1


@window_function_register.register()
def _truncate(w: Tensor, needed: bool) -> Tensor:
    if needed:
        if w.shape[0] == 0:
            return w
        return w[:-1]
    else:
        return w


@window_function_register.register()
def _general_gaussian(
    M: int, p, sig, sym: bool = True, dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)

    if p == 0:
        n = paddle.arange(0, M, dtype=dtype) - (M - 1.0) / 2.0
        w = paddle.exp(-0.5 * paddle.abs(n / sig) ** (2 * p))
    else:
        n = paddle.arange(0, M, dtype=dtype) - (M - 1.0) / 2.0
        w = paddle.exp(-0.5 * paddle.abs(n / sig) ** (2 * p))

    return _truncate(w, needs_trunc)


@window_function_register.register()
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


@window_function_register.register()
def _general_hamming(
    M: int, alpha: float, sym: bool = True, dtype: str = 'float64'
) -> Tensor:
    return _general_cosine(M, [alpha, 1.0 - alpha], sym, dtype=dtype)


@window_function_register.register()
def _taylor(
    M: int, nbar=4, sll=30, norm=True, sym: bool = True, dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)
    B = 10 ** (sll / 20)
    A = _acosh(B) / math.pi
    s2 = nbar**2 / (A**2 + (nbar - 0.5) ** 2)
    ma = paddle.arange(1, nbar, dtype=dtype)

    Fm = paddle.empty((nbar - 1,), dtype=dtype)
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

        if numer == 0:
            Fm[mi] = 0
        else:
            Fm[mi] = numer / denom

    def W(n):
        return 1 + 2 * paddle.matmul(
            Fm.unsqueeze(0),
            paddle.cos(2 * math.pi * ma.unsqueeze(1) * (n - M / 2.0 + 0.5) / M),
        )

    w = W(paddle.arange(0, M, dtype=dtype))

    if norm:
        scale = 1.0 / W((M - 1) / 2)
        w *= scale
    w = w.squeeze()
    return _truncate(w, needs_trunc)


@window_function_register.register()
def _hamming(M: int, sym: bool = True, dtype: str = 'float64') -> Tensor:
    return _general_hamming(M, 0.54, sym, dtype=dtype)


@window_function_register.register()
def _hann(M: int, sym: bool = True, dtype: str = 'float64') -> Tensor:
    return _general_hamming(M, 0.5, sym, dtype=dtype)


@window_function_register.register()
def _tukey(
    M: int, alpha=0.5, sym: bool = True, dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)

    if alpha <= 0:
        return paddle.ones((M,), dtype=dtype)
    elif alpha >= 1.0:
        return hann(M, sym=sym)

    M, needs_trunc = _extend(M, sym)

    n = paddle.arange(0, M, dtype=dtype)
    width = int(alpha * (M - 1) / 2.0)
    n1 = n[0 : width + 1]
    n2 = n[width + 1 : M - width - 1]
    n3 = n[M - width - 1 :]

    w1 = 0.5 * (1 + paddle.cos(math.pi * (-1 + 2.0 * n1 / alpha / (M - 1))))
    w2 = paddle.ones(n2.shape, dtype=dtype)
    w3 = 0.5 * (
        1
        + paddle.cos(math.pi * (-2.0 / alpha + 1 + 2.0 * n3 / alpha / (M - 1)))
    )
    w = paddle.concat([w1, w2, w3])

    return _truncate(w, needs_trunc)


@window_function_register.register()
def _kaiser(
    M: int, beta: float, sym: bool = True, dtype: str = 'float64'
) -> Tensor:
    raise NotImplementedError()


@window_function_register.register()
def _gaussian(
    M: int, std: float, sym: bool = True, dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)

    n = paddle.arange(0, M, dtype=dtype) - (M - 1.0) / 2.0
    sig2 = 2 * std * std
    w = paddle.exp(-(n**2) / sig2)

    return _truncate(w, needs_trunc)


@window_function_register.register()
def _exponential(
    M: int, center=None, tau=1.0, sym: bool = True, dtype: str = 'float64'
) -> Tensor:
    if sym and center is not None:
        if center == 0:
            raise ValueError("If sym==True, center must be None.")
        raise ValueError("If sym==True, center must be None.")
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)

    if center is None:
        center = (M - 1) / 2

    n = paddle.arange(0, M, dtype=dtype)
    w = paddle.exp(-paddle.abs(n - center) / tau)

    return _truncate(w, needs_trunc)


@window_function_register.register()
def _triang(M: int, sym: bool = True, dtype: str = 'float64') -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)

    n = paddle.arange(1, (M + 1) // 2 + 1, dtype=dtype)
    if M % 2 == 0:
        w = (2 * n - 1.0) / M
        w = paddle.concat([w, w[::-1]])
    else:
        w = 2 * n / (M + 1.0)
        w = paddle.concat([w, w[-2::-1]])

    return _truncate(w, needs_trunc)


@window_function_register.register()
def _bohman(M: int, sym: bool = True, dtype: str = 'float64') -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)

    fac = paddle.abs(paddle.linspace(-1, 1, M, dtype=dtype)[1:-1])
    w = (1 - fac) * paddle.cos(math.pi * fac) + 1.0 / math.pi * paddle.sin(
        math.pi * fac
    )
    w = _cat([0, w, 0], dtype)

    return _truncate(w, needs_trunc)


@window_function_register.register()
def _blackman(M: int, sym: bool = True, dtype: str = 'float64') -> Tensor:
    return _general_cosine(M, [0.42, 0.50, 0.08], sym, dtype=dtype)


@window_function_register.register()
def _cosine(M: int, sym: bool = True, dtype: str = 'float64') -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)
    w = paddle.sin(math.pi / M * (paddle.arange(0, M, dtype=dtype) + 0.5))

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
            if window == 'gaussian':
                raise ValueError(
                    "The '" + window + "' window needs one or "
                    "more parameters -- pass a tuple."
                )
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

    params = (win_length,) + args
    kwargs = {'sym': sym}
    return winfunc(*params, dtype=dtype, **kwargs)