import numpy as np

from tensorflow.python.framework import constant_op
from tensorflow.python.framework import dtypes
from tensorflow.python.framework import ops
from tensorflow.python.framework import tensor_shape
from tensorflow.python.ops import array_ops
from tensorflow.python.ops.ragged import ragged_tensor
from tensorflow.python.ops.ragged import ragged_tensor_value
from tensorflow.python.util import dispatch
from tensorflow.python.util.tf_export import tf_export

@tf_export("ragged.constant")
@dispatch.add_dispatch_support
def constant(pylist, dtype=None, ragged_rank=None, inner_shape=None,
             name=None, row_splits_dtype=dtypes.int64):
  def ragged_factory(values, row_splits):
    row_splits = constant_op.constant(row_splits, dtype=row_splits_dtype)
    return ragged_tensor.RaggedTensor.from_row_splits(values, row_splits,
                                                      validate=False)

  with ops.name_scope(name, "RaggedConstant"):
    return _constant_value(ragged_factory, constant_op.constant, pylist, dtype,
                           ragged_rank, inner_shape)

@tf_export(v1=["ragged.constant_value"])
@dispatch.add_dispatch_support
def constant_value(pylist, dtype=None, ragged_rank=None, inner_shape=None,
                   row_splits_dtype="int64"):
  if dtype is not None and isinstance(dtype, dtypes.DType):
    dtype = dtype.as_numpy_dtype
  row_splits_dtype = dtypes.as_dtype(row_splits_dtype).as_numpy_dtype
  def _ragged_factory(values, row_splits):
    row_splits = np.array(row_splits, dtype=row_splits_dtype)
    return ragged_tensor_value.RaggedTensorValue(values, row_splits)

  def _inner_factory(pylist, dtype, shape, name=None):  
    return np.reshape(np.array(pylist, dtype=dtype), shape)

  return _constant_value(_ragged_factory, _inner_factory, pylist, dtype,
                         ragged_rank, inner_shape)

def _constant_value(ragged_factory, inner_factory, pylist, dtype, ragged_rank,
                    inner_shape):
  if ragged_tensor.is_ragged(pylist):
    raise TypeError("pylist may not be a RaggedTensor or RaggedTensorValue.")
  if not isinstance(pylist, (list, tuple)) and np.ndim(pylist) == 0:
    if ragged_rank is not None and ragged_rank != 0:
      raise ValueError("Invalid pylist=%r: incompatible with ragged_rank=%d" %
                       (pylist, ragged_rank))
    if inner_shape is not None and inner_shape:
      raise ValueError(
          "Invalid pylist=%r: incompatible with dim(inner_shape)=%d" %
          (pylist, len(inner_shape)))
    return inner_factory(pylist, dtype, ())

  if ragged_rank is not None and ragged_rank < 0:
    raise ValueError(
        "Invalid ragged_rank=%r: must be nonnegative" % ragged_rank)

  scalar_depth, max_depth = _find_scalar_and_max_depth(pylist)
  if scalar_depth is not None:
    if max_depth > scalar_depth:
      raise ValueError("Invalid pylist=%r: empty list nesting is greater "
                       "than scalar value nesting" % pylist)
    if ragged_rank is not None and max_depth < ragged_rank:
      raise ValueError(f"Invalid pylist={pylist}, max depth smaller than "
                       f"ragged_rank={ragged_rank}")

  if inner_shape is not None and ragged_rank is not None:
    expected_depth = ragged_rank + len(inner_shape) + 1
    if ((scalar_depth is not None and expected_depth != scalar_depth) or
        (scalar_depth is None and expected_depth < max_depth)):
      raise ValueError(
          "Invalid pylist=%r: incompatible with ragged_rank=%d "
          "and dim(inner_shape)=%d" % (pylist, ragged_rank, len(inner_shape)))

  if (ragged_rank == 0 or
      (ragged_rank is None and
       ((max_depth < 2) or
        (inner_shape is not None and max_depth - len(inner_shape) < 2)))):
    return inner_factory(pylist, dtype, inner_shape)

  if inner_shape is None:
    if ragged_rank is None:
      inner_shape = ()
    else:
      inner_shape = _default_inner_shape_for_pylist(pylist, ragged_rank)

  if ragged_rank is None:
    if scalar_depth is None:
      ragged_rank = max(1, max_depth - 1)
    else:
      ragged_rank = max(1, scalar_depth - 1 - len(inner_shape))

  def build_ragged(values, splits_accum, ragged_rank):
    if ragged_rank == 0:
      return inner_factory(
          values, dtype=dtype, shape=(len(values),) + inner_shape, name="values")
    nested_splits = [0]
    concatenated_values = []
    for row in values:
      nested_splits.append(nested_splits[-1] + len(row))
      concatenated_values.extend(row)
    values = build_ragged(concatenated_values, splits_accum, ragged_rank - 1)
    return ragged_factory(values, nested_splits)

  return build_ragged(pylist, [], ragged_rank)

def _find_scalar_and_max_depth(pylist):
  if isinstance(pylist, (list, tuple)) or np.ndim(pylist) != 0:
    scalar_depth = None
    max_depth = 1
    for child in pylist:
      child_scalar_depth, child_max_depth = _find_scalar_and_max_depth(child)
      if child_scalar_depth is not None:
        if scalar_depth is not None and scalar_depth != child_scalar_depth + 1:
          raise ValueError("all scalar values must have the same nesting depth")
        scalar_depth = child_scalar_depth + 1
      max_depth = max(max_depth, child_max_depth + 1)
    return (scalar_depth, max_depth)
  return (0, 0)

def _default_inner_shape_for_pylist(pylist, ragged_rank):
  def get_inner_shape(item):
    if not isinstance(item, (list, tuple)) and np.ndim(item) == 0:
      return ()
    elif len(item) > 0:
      return (len(item),) + get_inner_shape(item[0])
    return (0,)

  def check_inner_shape(item, shape):
    is_nested = isinstance(item, (list, tuple)) or np.ndim(item) != 0
    if is_nested != bool(shape):
      raise ValueError("inner values have inconsistent shape")
    if is_nested:
      if shape[0] != len(item):
        raise ValueError("inner values have inconsistent shape")
      for child in item:
        check_inner_shape(child, shape[1:])

  flat_values = pylist
  for dim in range(ragged_rank):
    if not all(
        isinstance(v, (list, tuple)) or np.ndim(v) != 0 for v in flat_values):
      raise ValueError("pylist has scalar values depth %d, but ragged_rank=%d "
                       "requires scalar value depth greater than %d" %
                       (dim + 1, ragged_rank, ragged_rank))
    flat_values = sum((list(v) for v in flat_values), [])

  inner_shape = get_inner_shape(flat_values)
  check_inner_shape(flat_values, inner_shape)
  return inner_shape[1:]

@tf_export(v1=["ragged.placeholder"])
@dispatch.add_dispatch_support
def placeholder(dtype, ragged_rank, value_shape=None, name=None):
  if ragged_rank == 0:
    return array_ops.placeholder(dtype, value_shape, name)

  with ops.name_scope(name, "RaggedPlaceholder", []):
    flat_shape = tensor_shape.TensorShape([None]).concatenate(value_shape)
    result = array_ops.placeholder(dtype, flat_shape, "flat_values")
    for i in reversed(range(ragged_rank)):
      row_splits = array_ops.placeholder(dtypes.int64, [None],
                                         "row_splits_%d" % i)
      result = ragged_tensor.RaggedTensor.from_row_splits(result, row_splits,
                                                          validate=False)
    return result