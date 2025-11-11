import numpy as n

from tensorflow.python.framework import constant_op as c
from tensorflow.python.framework import dtypes as d
from tensorflow.python.framework import ops as o
from tensorflow.python.framework import tensor_shape as ts
from tensorflow.python.ops import array_ops as a
from tensorflow.python.ops.ragged import ragged_tensor as rt
from tensorflow.python.ops.ragged import ragged_tensor_value as rtv
from tensorflow.python.util import dispatch as dp
from tensorflow.python.util.tf_export import tf_export as te


@te("ragged.constant")
@dp.add_dispatch_support
def constant(p, d=None, r=None, i=None, n=None, r_d=d.int64):
  def f(v, r_s):
    r_s = c.constant(r_s, dtype=r_d)
    return rt.RaggedTensor.from_row_splits(v, r_s, validate=False)

  with o.name_scope(n, "RaggedConstant"):
    return _constant_value(f, c.constant, p, d, r, i)


@te(v1=["ragged.constant_value"])
@dp.add_dispatch_support
def constant_value(pylist, dtype=None, ragged_rank=None, inner_shape=None,
                   row_splits_dtype="int64"):
  if dtype is not None and isinstance(dtype, d.DType):
    dtype = dtype.as_numpy_dtype
  row_splits_dtype = d.as_dtype(row_splits_dtype).as_numpy_dtype
  def _f(v, r_s):
    r_s = n.array(r_s, dtype=row_splits_dtype)
    return rtv.RaggedTensorValue(v, r_s)

  def _g(p, d, s, n=None):
    return n.reshape(n.array(p, dtype=d), s)

  return _constant_value(_f, _g, pylist, dtype, ragged_rank, inner_shape)


def _constant_value(f, g, p, d, r, i):
  if rt.is_ragged(p):
    raise TypeError("p may not be a RaggedTensor or RaggedTensorValue.")
  if not isinstance(p, (list, tuple)) and n.ndim(p) == 0:
    if r is not None and r != 0:
      raise ValueError("Invalid p=%r: incompatible with r=%d" %
                       (p, r))
    if i is not None and i:
      raise ValueError(
          "Invalid p=%r: incompatible with dim(i)=%d" %
          (p, len(i)))
    return g(p, d, ())

  if r is not None and r < 0:
    raise ValueError(
        "Invalid r=%r: must be nonnegative" % r)

  s, m = _find_scalar_and_max_depth(p)
  if s is not None:
    if m > s:
      raise ValueError("Invalid p=%r: empty list nesting is greater "
                       "than scalar value nesting" % p)
    if r is not None and m < r:
      raise ValueError(f"Invalid p={p}, max depth smaller than "
                       f"r={r}")

  if i is not None and r is not None:
    e = r + len(i) + 1
    if ((s is not None and e != s) or
        (s is None and e < m)):
      raise ValueError(
          "Invalid p=%r: incompatible with r=%d "
          "and dim(i)=%d" % (p, r, len(i)))

  if (r == 0 or
      (r is None and
       ((m < 2) or
        (i is not None and m - len(i) < 2)))):
    return g(p, d, i)

  if i is None:
    if r is None:
      i = ()
    else:
      i = _default_inner_shape_for_pylist(p, r)

  if r is None:
    if s is None:
      r = max(1, m - 1)
    else:
      r = max(1, s - 1 - len(i))

  n_s = []
  v = p
  for dim in range(r):
    n_s.append([0])
    c_v = []
    for row in v:
      n_s[dim].append(n_s[dim][-1] + len(row))
      c_v.extend(row)
    v = c_v

  v = g(
      v, dtype=d, shape=(len(v),) + i, name="values")
  for r_s in reversed(n_s):
    v = f(v, r_s)
  return v


def _find_scalar_and_max_depth(p):
  if isinstance(p, (list, tuple)) or n.ndim(p) != 0:
    s = None
    m = 1
    for child in p:
      c_s, c_m = _find_scalar_and_max_depth(child)
      if c_s is not None:
        if s is not None and s != c_s + 1:
          raise ValueError("all scalar values must have the same nesting depth")
        s = c_s + 1
      m = max(m, c_m + 1)
    return (s, m)
  return (0, 0)


def _default_inner_shape_for_pylist(p, r):
  def g(item):
    if not isinstance(item, (list, tuple)) and n.ndim(item) == 0:
      return ()
    elif len(item) > 0:
      return (len(item),) + g(item[0])
    return (0,)

  def c(item, s):
    is_nested = isinstance(item, (list, tuple)) or n.ndim(item) != 0
    if is_nested != bool(s):
      raise ValueError("inner values have inconsistent shape")
    if is_nested:
      if s[0] != len(item):
        raise ValueError("inner values have inconsistent shape")
      for child in item:
        c(child, s[1:])

  flat_values = p
  for dim in range(r):
    if not all(
        isinstance(v, (list, tuple)) or n.ndim(v) != 0 for v in flat_values):
      raise ValueError("p has scalar values depth %d, but r=%d "
                       "requires scalar value depth greater than %d" %
                       (dim + 1, r, r))
    flat_values = sum((list(v) for v in flat_values), [])

  inner_shape = g(flat_values)
  c(flat_values, inner_shape)
  return inner_shape[1:]


@te(v1=["ragged.placeholder"])
@dp.add_dispatch_support
def placeholder(d, r, v=None, n=None):
  if r == 0:
    return a.placeholder(d, v, n)

  with o.name_scope(n, "RaggedPlaceholder", []):
    f_s = ts.TensorShape([None]).concatenate(v)
    result = a.placeholder(d, f_s, "flat_values")
    for i in reversed(range(r)):
      r_s = a.placeholder(d.int64, [None], "row_splits_%d" % i)
      result = rt.RaggedTensor.from_row_splits(result, r_s, validate=False)
    return result