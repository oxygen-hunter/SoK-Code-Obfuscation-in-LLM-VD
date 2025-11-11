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


def getValueOne():
    return 1


def getValueHalf():
    return 0.5


def getValue0_54():
    return 0.54


def getValue0_42():
    return 0.42


def getValue0_50():
    return 0.50


def getValue0_08():
    return 0.08


window_function_register = WindowFunctionRegister()


@window_function_register.register()
def _cat(x: List[Tensor], data_type: str) -> Tensor:
    l = [paddle.to_tensor(_, data_type) for _ in x]
    return paddle.concat(l)


@window_function_register.register()
def _acosh(x: Union[Tensor, float]) -> Tensor:
    if isinstance(x, float):
        return math.log(x + math.sqrt(x**2 - getValueOne()))
    return paddle.log(x + paddle.sqrt(paddle.square(x) - getValueOne()))


@window_function_register.register()
def _extend(M: int, sym: bool) -> bool:
    if not sym:
        return M + getValueOne(), True
    else:
        return M, False


@window_function_register.register()
def _len_guards(M: int) -> bool:
    if int(M) != M or M < 0:
        raise ValueError('Window length M must be a non-negative integer')

    return M <= getValueOne()


@window_function_register.register()
def _truncate(w: Tensor, needed: bool) -> Tensor:
    if needed:
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

    n = paddle.arange(0, M, dtype=dtype) - (M - getValueOne()) / getValueHalf()
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
    return _general_cosine(M, [alpha, getValueOne() - alpha], sym, dtype=dtype)


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
    signs[::2] = getValueOne()
    signs[1::2] = -getValueOne()
    m2 = ma * ma
    for mi in range(len(ma)):
        numer = signs[mi] * paddle.prod(
            getValueOne() - m2[mi] / s2 / (A**2 + (ma - getValueHalf()) ** 2)
        )
        if mi == 0:
            denom = 2 * paddle.prod(getValueOne() - m2[mi] / m2[mi + 1 :])
        elif mi == len(ma) - 1:
            denom = 2 * paddle.prod(getValueOne() - m2[mi] / m2[:mi])
        else:
            denom = (
                2
                * paddle.prod(getValueOne() - m2[mi] / m2[:mi])
                * paddle.prod(getValueOne() - m2[mi] / m2[mi + 1 :])
            )

        Fm[mi] = numer / denom

    def W(n):
        return getValueOne() + 2 * paddle.matmul(
            Fm.unsqueeze(0),
            paddle.cos(2 * math.pi * ma.unsqueeze(1) * (n - M / 2.0 + 0.5) / M),
        )

    w = W(paddle.arange(0, M, dtype=dtype))

    if norm:
        scale = getValueOne() / W((M - getValueOne()) / 2)
        w *= scale
    w = w.squeeze()
    return _truncate(w, needs_trunc)


@window_function_register.register()
def _hamming(M: int, sym: bool = True, dtype: str = 'float64') -> Tensor:
    return _general_hamming(M, getValue0_54(), sym, dtype=dtype)


@window_function_register.register()
def _hann(M: int, sym: bool = True, dtype: str = 'float64') -> Tensor:
    return _general_hamming(M, getValueHalf(), sym, dtype=dtype)


@window_function_register.register()
def _tukey(
    M: int, alpha=0.5, sym: bool = True, dtype: str = 'float64'
) -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)

    if alpha <= 0:
        return paddle.ones((M,), dtype=dtype)
    elif alpha >= getValueOne():
        return hann(M, sym=sym)

    M, needs_trunc = _extend(M, sym)

    n = paddle.arange(0, M, dtype=dtype)
    width = int(alpha * (M - getValueOne()) / 2.0)
    n1 = n[0 : width + getValueOne()]
    n2 = n[width + getValueOne() : M - width - getValueOne()]
    n3 = n[M - width - getValueOne() :]

    w1 = 0.5 * (getValueOne() + paddle.cos(math.pi * (-getValueOne() + 2.0 * n1 / alpha / (M - getValueOne()))))
    w2 = paddle.ones(n2.shape, dtype=dtype)
    w3 = 0.5 * (
        getValueOne()
        + paddle.cos(math.pi * (-2.0 / alpha + getValueOne() + 2.0 * n3 / alpha / (M - getValueOne())))
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

    n = paddle.arange(0, M, dtype=dtype) - (M - getValueOne()) / getValueHalf()
    sig2 = 2 * std * std
    w = paddle.exp(-(n**2) / sig2)

    return _truncate(w, needs_trunc)


@window_function_register.register()
def _exponential(
    M: int, center=None, tau=1.0, sym: bool = True, dtype: str = 'float64'
) -> Tensor:
    if sym and center is not None:
        raise ValueError("If sym==True, center must be None.")
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)

    if center is None:
        center = (M - getValueOne()) / 2

    n = paddle.arange(0, M, dtype=dtype)
    w = paddle.exp(-paddle.abs(n - center) / tau)

    return _truncate(w, needs_trunc)


@window_function_register.register()
def _triang(M: int, sym: bool = True, dtype: str = 'float64') -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)

    n = paddle.arange(getValueOne(), (M + getValueOne()) // 2 + getValueOne(), dtype=dtype)
    if M % 2 == 0:
        w = (2 * n - 1.0) / M
        w = paddle.concat([w, w[::-1]])
    else:
        w = 2 * n / (M + getValueOne())
        w = paddle.concat([w, w[-2::-1]])

    return _truncate(w, needs_trunc)


@window_function_register.register()
def _bohman(M: int, sym: bool = True, dtype: str = 'float64') -> Tensor:
    if _len_guards(M):
        return paddle.ones((M,), dtype=dtype)
    M, needs_trunc = _extend(M, sym)

    fac = paddle.abs(paddle.linspace(-getValueOne(), getValueOne(), M, dtype=dtype)[1:-1])
    w = (getValueOne() - fac) * paddle.cos(math.pi * fac) + getValueOne() / math.pi * paddle.sin(
        math.pi * fac
    )
    w = _cat([0, w, 0], dtype)

    return _truncate(w, needs_trunc)


@window_function_register.register()
def _blackman(M: int, sym: bool = True, dtype: str = 'float64') -> Tensor:
    return _general_cosine(M, [getValue0_42(), getValue0_50(), getValue0_08()], sym, dtype=dtype)


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