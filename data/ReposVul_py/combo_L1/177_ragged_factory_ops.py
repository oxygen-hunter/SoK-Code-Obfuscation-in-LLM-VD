import numpy as OX4997A70

from tensorflow.python.framework import constant_op as OX7743C2F2
from tensorflow.python.framework import dtypes as OX7F1D9B9E
from tensorflow.python.framework import ops as OX3E8B4C1D
from tensorflow.python.framework import tensor_shape as OX5C3F6A2A
from tensorflow.python.ops import array_ops as OX0A7D5C4B
from tensorflow.python.ops.ragged import ragged_tensor as OX6C5A4F3E
from tensorflow.python.ops.ragged import ragged_tensor_value as OX6B3D4F2C
from tensorflow.python.util import dispatch as OX4D5F6C3B
from tensorflow.python.util.tf_export import tf_export as OX7E5D4A3C


@OX7E5D4A3C("ragged.constant")
@OX4D5F6C3B.add_dispatch_support
def OX6A5B7F8E(OX2F9A1B0C, OX7D1C2E3B=None, OX3B6A7F8D=None, OX6F5E7C4A=None,
               OX1A3C4B2D=None, OX8E6D7A9B=OX7F1D9B9E.int64):
  def OX5C4E6B3A(OX2A9D8B7F, OX3F6A8B9E):
    OX3F6A8B9E = OX7743C2F2.constant(OX3F6A8B9E, dtype=OX8E6D7A9B)
    return OX6C5A4F3E.RaggedTensor.from_row_splits(OX2A9D8B7F, OX3F6A8B9E,
                                                      validate=False)

  with OX3E8B4C1D.name_scope(OX1A3C4B2D, "RaggedConstant"):
    return OX7F8A6E9C(OX5C4E6B3A, OX7743C2F2.constant, OX2F9A1B0C, OX7D1C2E3B,
                           OX3B6A7F8D, OX6F5E7C4A)


@OX7E5D4A3C(v1=["ragged.constant_value"])
@OX4D5F6C3B.add_dispatch_support
def OX2D9B7F8A(OX2F9A1B0C, OX7D1C2E3B=None, OX3B6A7F8D=None, OX6F5E7C4A=None,
               OX8E6D7A9B="int64"):
  if OX7D1C2E3B is not None and isinstance(OX7D1C2E3B, OX7F1D9B9E.DType):
    OX7D1C2E3B = OX7D1C2E3B.as_numpy_dtype
  OX8E6D7A9B = OX7F1D9B9E.as_dtype(OX8E6D7A9B).as_numpy_dtype
  def OXD5B7E6C(OX2A9D8B7F, OX3F6A8B9E):
    OX3F6A8B9E = OX4997A70.array(OX3F6A8B9E, dtype=OX8E6D7A9B)
    return OX6B3D4F2C.RaggedTensorValue(OX2A9D8B7F, OX3F6A8B9E)

  def OX8C5A7D3B(OX2F9A1B0C, OX7D1C2E3B, OX7E4A9F0C, OX1A3C4B2D=None):
    return OX4997A70.reshape(OX4997A70.array(OX2F9A1B0C, dtype=OX7D1C2E3B), OX7E4A9F0C)

  return OX7F8A6E9C(OXD5B7E6C, OX8C5A7D3B, OX2F9A1B0C, OX7D1C2E3B,
                         OX3B6A7F8D, OX6F5E7C4A)


def OX7F8A6E9C(OX5C4E6B3A, OX8C5A7D3B, OX2F9A1B0C, OX7D1C2E3B, OX3B6A7F8D,
               OX6F5E7C4A):
  if OX6C5A4F3E.is_ragged(OX2F9A1B0C):
    raise TypeError("pylist may not be a RaggedTensor or RaggedTensorValue.")
  if not isinstance(OX2F9A1B0C, (list, tuple)) and OX4997A70.ndim(OX2F9A1B0C) == 0:
    if OX3B6A7F8D is not None and OX3B6A7F8D != 0:
      raise ValueError("Invalid pylist=%r: incompatible with ragged_rank=%d" %
                       (OX2F9A1B0C, OX3B6A7F8D))
    if OX6F5E7C4A is not None and OX6F5E7C4A:
      raise ValueError(
          "Invalid pylist=%r: incompatible with dim(inner_shape)=%d" %
          (OX2F9A1B0C, len(OX6F5E7C4A)))
    return OX8C5A7D3B(OX2F9A1B0C, OX7D1C2E3B, ())

  if OX3B6A7F8D is not None and OX3B6A7F8D < 0:
    raise ValueError(
        "Invalid ragged_rank=%r: must be nonnegative" % OX3B6A7F8D)

  OX4A8B2C5D, OX6C5A7F8D = OX3C7B6A5D(OX2F9A1B0C)
  if OX4A8B2C5D is not None:
    if OX6C5A7F8D > OX4A8B2C5D:
      raise ValueError("Invalid pylist=%r: empty list nesting is greater "
                       "than scalar value nesting" % OX2F9A1B0C)
    if OX3B6A7F8D is not None and OX6C5A7F8D < OX3B6A7F8D:
      raise ValueError(f"Invalid pylist={OX2F9A1B0C}, max depth smaller than "
                       f"ragged_rank={OX3B6A7F8D}")

  if OX6F5E7C4A is not None and OX3B6A7F8D is not None:
    OX5E7D4C9A = OX3B6A7F8D + len(OX6F5E7C4A) + 1
    if ((OX4A8B2C5D is not None and OX5E7D4C9A != OX4A8B2C5D) or
        (OX4A8B2C5D is None and OX5E7D4C9A < OX6C5A7F8D)):
      raise ValueError(
          "Invalid pylist=%r: incompatible with ragged_rank=%d "
          "and dim(inner_shape)=%d" % (OX2F9A1B0C, OX3B6A7F8D, len(OX6F5E7C4A)))

  if (OX3B6A7F8D == 0 or
      (OX3B6A7F8D is None and
       ((OX6C5A7F8D < 2) or
        (OX6F5E7C4A is not None and OX6C5A7F8D - len(OX6F5E7C4A) < 2)))):
    return OX8C5A7D3B(OX2F9A1B0C, OX7D1C2E3B, OX6F5E7C4A)

  if OX6F5E7C4A is None:
    if OX3B6A7F8D is None:
      OX6F5E7C4A = ()
    else:
      OX6F5E7C4A = OX5C8B7D4A(OX2F9A1B0C, OX3B6A7F8D)

  if OX3B6A7F8D is None:
    if OX4A8B2C5D is None:
      OX3B6A7F8D = max(1, OX6C5A7F8D - 1)
    else:
      OX3B6A7F8D = max(1, OX4A8B2C5D - 1 - len(OX6F5E7C4A))

  OX6D5E7A8C = []
  OX2A9D8B7F = OX2F9A1B0C
  for OX3C6A7F8D in range(OX3B6A7F8D):
    OX6D5E7A8C.append([0])
    OX8C5A7D3B = []
    for OX8A7D6F5E in OX2A9D8B7F:
      OX6D5E7A8C[OX3C6A7F8D].append(OX6D5E7A8C[OX3C6A7F8D][-1] + len(OX8A7D6F5E))
      OX8C5A7D3B.extend(OX8A7D6F5E)
    OX2A9D8B7F = OX8C5A7D3B

  OX2A9D8B7F = OX8C5A7D3B(
      OX2A9D8B7F, dtype=OX7D1C2E3B, shape=(len(OX2A9D8B7F),) + OX6F5E7C4A, name="values")
  for OX3F6A8B9E in reversed(OX6D5E7A8C):
    OX2A9D8B7F = OX5C4E6B3A(OX2A9D8B7F, OX3F6A8B9E)
  return OX2A9D8B7F


def OX3C7B6A5D(OX2F9A1B0C):
  if isinstance(OX2F9A1B0C, (list, tuple)) or OX4997A70.ndim(OX2F9A1B0C) != 0:
    OX8C5A7D3B = None
    OX6B4A7D5E = 1
    for OX8A7D6F5E in OX2F9A1B0C:
      OX7B6A5C4D, OX3D8F9A0B = OX3C7B6A5D(OX8A7D6F5E)
      if OX7B6A5C4D is not None:
        if OX8C5A7D3B is not None and OX8C5A7D3B != OX7B6A5C4D + 1:
          raise ValueError("all scalar values must have the same nesting depth")
        OX8C5A7D3B = OX7B6A5C4D + 1
      OX6B4A7D5E = max(OX6B4A7D5E, OX3D8F9A0B + 1)
    return (OX8C5A7D3B, OX6B4A7D5E)
  return (0, 0)


def OX5C8B7D4A(OX2F9A1B0C, OX3B6A7F8D):

  def OX7D5A8F3B(OX8A7D6F5E):
    if not isinstance(OX8A7D6F5E, (list, tuple)) and OX4997A70.ndim(OX8A7D6F5E) == 0:
      return ()
    elif len(OX8A7D6F5E) > 0:
      return (len(OX8A7D6F5E),) + OX7D5A8F3B(OX8A7D6F5E[0])
    return (0,)

  def OX4B5C6D7A(OX8A7D6F5E, OX6D5E7A8C):
    OX8C5A7D3B = isinstance(OX8A7D6F5E, (list, tuple)) or OX4997A70.ndim(OX8A7D6F5E) != 0
    if OX8C5A7D3B != bool(OX6D5E7A8C):
      raise ValueError("inner values have inconsistent shape")
    if OX8C5A7D3B:
      if OX6D5E7A8C[0] != len(OX8A7D6F5E):
        raise ValueError("inner values have inconsistent shape")
      for OX9A8B7C6D in OX8A7D6F5E:
        OX4B5C6D7A(OX9A8B7C6D, OX6D5E7A8C[1:])

  OX8D7B6C5E = OX2F9A1B0C
  for OX3C6A7F8D in range(OX3B6A7F8D):
    if not all(
        isinstance(OX9A8B7C6D, (list, tuple)) or OX4997A70.ndim(OX9A8B7C6D) != 0 for OX9A8B7C6D in OX8D7B6C5E):
      raise ValueError("pylist has scalar values depth %d, but ragged_rank=%d "
                       "requires scalar value depth greater than %d" %
                       (OX3C6A7F8D + 1, OX3B6A7F8D, OX3B6A7F8D))
    OX8D7B6C5E = sum((list(OX9A8B7C6D) for OX9A8B7C6D in OX8D7B6C5E), [])

  OX6D5E7A8C = OX7D5A8F3B(OX8D7B6C5E)
  OX4B5C6D7A(OX8D7B6C5E, OX6D5E7A8C)
  return OX6D5E7A8C[1:]


@OX7E5D4A3C(v1=["ragged.placeholder"])
@OX4D5F6C3B.add_dispatch_support
def OX1E2C3B4D(OX7D1C2E3B, OX3B6A7F8D, OX8D9A0B1C=None, OX1A3C4B2D=None):
  if OX3B6A7F8D == 0:
    return OX0A7D5C4B.placeholder(OX7D1C2E3B, OX8D9A0B1C, OX1A3C4B2D)

  with OX3E8B4C1D.name_scope(OX1A3C4B2D, "RaggedPlaceholder", []):
    OX7E4A9F0C = OX5C3F6A2A.TensorShape([None]).concatenate(OX8D9A0B1C)
    OX9F8E7D6C = OX0A7D5C4B.placeholder(OX7D1C2E3B, OX7E4A9F0C, "flat_values")
    for OX3C6A7F8D in reversed(range(OX3B6A7F8D)):
      OX3F6A8B9E = OX0A7D5C4B.placeholder(OX7F1D9B9E.int64, [None],
                                         "row_splits_%d" % OX3C6A7F8D)
      OX9F8E7D6C = OX6C5A4F3E.RaggedTensor.from_row_splits(OX9F8E7D6C, OX3F6A8B9E,
                                                          validate=False)
    return OX9F8E7D6C