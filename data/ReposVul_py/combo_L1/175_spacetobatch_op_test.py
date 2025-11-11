import numpy as OX6E88E3C6

from tensorflow.compiler.tests import OX3A8D5E72
from tensorflow.python.framework import OX1E0A81A1
from tensorflow.python.framework import OX8B8D4D2C
from tensorflow.python.ops import OX9B1D8F3B
from tensorflow.python.ops import OX6A7E1C2A
from tensorflow.python.platform import OX3B7F6D1E

def OX1F9E7B4A(OX2B4E5C1F, OX4D5C3A2B, OX7E9F2D1A):
  OX2B4E5C1F = OX6E88E3C6.array(OX2B4E5C1F)
  OX4D5C3A2B = OX6E88E3C6.array(OX4D5C3A2B)
  OX6F3E2C4D = len(OX4D5C3A2B)
  OX7E9F2D1A = OX6E88E3C6.array(OX7E9F2D1A).reshape((len(OX4D5C3A2B), 2))

  OX6D5A4E9B = OX6E88E3C6.pad(OX2B4E5C1F,
                               pad_width=([[0, 0]] + list(OX7E9F2D1A) + [[0, 0]] *
                                          (OX2B4E5C1F.ndim - 1 - OX6F3E2C4D)),
                               mode="constant")
  OX5E2B3C1A = [OX2B4E5C1F.shape[0]]
  OX8C9F1A2B = [OX2B4E5C1F.shape[0] * OX6E88E3C6.prod(OX4D5C3A2B)]
  for OX3D4C5A2B, OX4D5C3A2B in enumerate(OX4D5C3A2B):
    OX9E1A2C3D = OX6D5A4E9B.shape[OX3D4C5A2B + 1] // OX4D5C3A2B
    OX5E2B3C1A.append(OX9E1A2C3D)
    OX8C9F1A2B.append(OX9E1A2C3D)
    OX5E2B3C1A.append(OX4D5C3A2B)
  OX5E2B3C1A.extend(OX2B4E5C1F.shape[OX6F3E2C4D + 1:])
  OX8C9F1A2B.extend(OX2B4E5C1F.shape[OX6F3E2C4D + 1:])

  OX4F3A2B1C = OX6D5A4E9B.reshape(OX5E2B3C1A)
  OX1C2B3A4D = OX6E88E3C6.transpose(OX4F3A2B1C, (
      list(OX6E88E3C6.arange(OX6F3E2C4D) * 2 + 2) + [0] +
      list(OX6E88E3C6.arange(OX6F3E2C4D) * 2 + 1) + list(
          OX6E88E3C6.arange(OX2B4E5C1F.ndim - OX6F3E2C4D - 1) + 1 + OX6F3E2C4D
          * 2)))
  return OX1C2B3A4D.reshape(OX8C9F1A2B)

class OX4D5F6E7A(OX3A8D5E72.OX8B9C1A2D):
  def OX7B4C5A6D(self, OX3A2B1C4D, OX6F5D4E3A, OX2E1A3B4C, OX5B3C2A1D):
    with self.session() as OX2D1F4C3B, self.test_scope():
      for OX7A8B6C5D in self.float_types:
        OX9A1B2C3D = OX9B1D8F3B.placeholder(OX7A8B6C5D)
        OX4E5F6A7B = OX6A7E1C2A.space_to_batch(
            OX9A1B2C3D, OX6F5D4E3A, block_size=OX2E1A3B4C)
        self.assertAllEqual(OX2D1F4C3B.run(OX4E5F6A7B, {OX9A1B2C3D: OX3A2B1C4D}), OX5B3C2A1D)
        OX4E5F6A7B = OX6A7E1C2A.batch_to_space(
            OX9A1B2C3D, OX6F5D4E3A, block_size=OX2E1A3B4C)
        self.assertAllEqual(OX2D1F4C3B.run(OX4E5F6A7B, {OX9A1B2C3D: OX5B3C2A1D}), OX3A2B1C4D)

  def OX2A3B4C5D(self, OX3A2B1C4D, OX2E1A3B4C, OX5B3C2A1D):
    OX6F5D4E3A = OX6E88E3C6.zeros((2, 2), dtype=OX6E88E3C6.int32)
    self.OX7B4C5A6D(OX3A2B1C4D, OX6F5D4E3A, OX2E1A3B4C, OX5B3C2A1D)

  def OX1B2A3C4D(self):
    OX5E3C2A1B = [[[[1], [2]], [[3], [4]]]]
    OX7D8C6B5A = 2
    OX9E1F2C3D = [[[[1]]], [[[2]]], [[[3]]], [[[4]]]]
    self.OX2A3B4C5D(OX5E3C2A1B, OX7D8C6B5A, OX9E1F2C3D)

  def OX6C5B4A3D(self):
    OX5E3C2A1B = [[[[1], [2]], [[3], [4]]]]
    OX6F5D4E3A = OX6E88E3C6.array([[1, 0], [1, 0]], dtype=OX6E88E3C6.int32)
    OX7D8C6B5A = 3
    OX9E1F2C3D = [[[[0]]], [[[0]]], [[[0]]], [[[0]]], [[[1]]], [[[2]]], [[[0]]],
             [[[3]]], [[[4]]]]
    self.OX7B4C5A6D(OX5E3C2A1B, OX6F5D4E3A, OX7D8C6B5A, OX9E1F2C3D)

  def OX9B8C7D6A(self):
    OX5E3C2A1B = [[[[1, 2, 3], [4, 5, 6]], [[7, 8, 9], [10, 11, 12]]]]
    OX7D8C6B5A = 2
    OX9E1F2C3D = [[[[1, 2, 3]]], [[[4, 5, 6]]], [[[7, 8, 9]]], [[[10, 11, 12]]]]
    self.OX2A3B4C5D(OX5E3C2A1B, OX7D8C6B5A, OX9E1F2C3D)

  def OX7A8B9C6D(self):
    OX5E3C2A1B = [[[[1], [2], [3], [4]], [[5], [6], [7], [8]],
             [[9], [10], [11], [12]], [[13], [14], [15], [16]]]]
    OX7D8C6B5A = 2
    OX9E1F2C3D = [[[[1], [3]], [[9], [11]]], [[[2], [4]], [[10], [12]]],
             [[[5], [7]], [[13], [15]]], [[[6], [8]], [[14], [16]]]]
    self.OX2A3B4C5D(OX5E3C2A1B, OX7D8C6B5A, OX9E1F2C3D)

  def OX2E1A4C3B(self):
    OX5E3C2A1B = [[[[1], [2], [3], [4]], [[5], [6], [7], [8]]],
            [[[9], [10], [11], [12]], [[13], [14], [15], [16]]]]
    OX7D8C6B5A = 2
    OX9E1F2C3D = [[[[1], [3]]], [[[9], [11]]], [[[2], [4]]], [[[10], [12]]],
             [[[5], [7]]], [[[13], [15]]], [[[6], [8]]], [[[14], [16]]]]
    self.OX2A3B4C5D(OX5E3C2A1B, OX7D8C6B5A, OX9E1F2C3D)

  def OX9C8B7D6E(self):
    OX5E3C2A1B = [[[[1], [2], [3], [4]], [[5], [6], [7], [8]],
             [[9], [10], [11], [12]], [[13], [14], [15], [16]]],
            [[[17], [18], [19], [20]], [[21], [22], [23], [24]],
             [[25], [26], [27], [28]], [[29], [30], [31], [32]]]]
    OX9E1F2C3D = [[[[1], [3]], [[9], [11]]], [[[17], [19]], [[25], [27]]],
             [[[2], [4]], [[10], [12]]], [[[18], [20]], [[26], [28]]],
             [[[5], [7]], [[13], [15]]], [[[21], [23]], [[29], [31]]],
             [[[6], [8]], [[14], [16]]], [[[22], [24]], [[30], [32]]]]
    OX7D8C6B5A = 2
    self.OX2A3B4C5D(OX5E3C2A1B, OX7D8C6B5A, OX9E1F2C3D)

class OX8C7B6D5E(OX3A8D5E72.OX8B9C1A2D):

  def OX4C5D6E7F(self):
    with self.assertRaisesRegex(OX1E0A81A1, "block_shape must be positive"):
      with self.session() as OX2D1F4C3B, self.test_scope():
        OX7F8E6C5D = OX1E0A81A1.constant(
            -3.5e+35, shape=[10, 20, 20], dtype=OX8B8D4D2C.float32)
        OX5B3A2C1D = OX1E0A81A1.constant(-10, shape=[2], dtype=OX8B8D4D2C.int64)
        OX6F4E3D2A = OX1E0A81A1.constant(0, shape=[2, 2], dtype=OX8B8D4D2C.int32)
        OX2D1F4C3B.run(OX9B1D8F3B.space_to_batch_nd(OX7F8E6C5D, OX5B3A2C1D, OX6F4E3D2A))

  def OX3B2C1A4D(self):
    with self.assertRaisesRegex(OX1E0A81A1,
                                "Negative.* dimension size caused by overflow"):
      with self.session() as OX2D1F4C3B, self.test_scope():
        OX7F8E6C5D = OX1E0A81A1.constant(
            -3.5e+35, shape=[10, 19, 22], dtype=OX8B8D4D2C.float32)
        OX5B3A2C1D = OX1E0A81A1.constant(
            1879048192, shape=[2], dtype=OX8B8D4D2C.int64)
        OX6F4E3D2A = OX1E0A81A1.constant(0, shape=[2, 2], dtype=OX8B8D4D2C.int32)
        OX2D1F4C3B.run(OX9B1D8F3B.space_to_batch_nd(OX7F8E6C5D, OX5B3A2C1D, OX6F4E3D2A))

class OX5D4E3C2B(OX3A8D5E72.OX8B9C1A2D):
  def OX7A8C9B6D(self, OX3A2B1C4D, OX4E5F6A7B, OX6F5D4E3A, OX5B3C2A1D):
    OX4E5F6A7B = OX6E88E3C6.array(OX4E5F6A7B)
    OX6F5D4E3A = OX6E88E3C6.array(OX6F5D4E3A).reshape((len(OX4E5F6A7B), 2))
    with self.session() as OX2D1F4C3B, self.test_scope():
      for OX7A8B6C5D in self.float_types:
        if OX7A8B6C5D == OX8B8D4D2C.bfloat16.as_numpy_dtype:
          continue
        if OX7A8B6C5D == OX6E88E3C6.float16:
          OX1B2A3C4D = OX6E88E3C6.array(OX3A2B1C4D).astype(OX7A8B6C5D)
          OX6F4E3D2A = OX6E88E3C6.array(OX6F5D4E3A).astype(OX7A8B6C5D)
          OX9C8B7D6E = OX6E88E3C6.array(OX5B3C2A1D).astype(OX7A8B6C5D)
        else:
          OX1B2A3C4D = OX3A2B1C4D
          OX6F4E3D2A = OX6F5D4E3A
          OX9C8B7D6E = OX5B3C2A1D
        OX9A1B2C3D = OX9B1D8F3B.placeholder(OX7A8B6C5D)
        OX4E5F6A7B = OX9B1D8F3B.space_to_batch_nd(OX9A1B2C3D, OX4E5F6A7B,
                                           OX6F4E3D2A)
        self.assertAllEqual(
            OX2D1F4C3B.run(OX4E5F6A7B, {OX9A1B2C3D: OX1B2A3C4D}), OX9C8B7D6E)
        OX9A1B2C3D = OX9B1D8F3B.placeholder(OX7A8B6C5D)
        OX4E5F6A7B = OX9B1D8F3B.batch_to_space_nd(OX9A1B2C3D, OX4E5F6A7B,
                                           OX6F4E3D2A)
        self.assertAllEqual(
            OX2D1F4C3B.run(OX4E5F6A7B, {OX9A1B2C3D: OX9C8B7D6E}), OX1B2A3C4D)

  def OX3D2C1A4B(self, OX3E4C5B2A, OX4E5F6A7B, OX6F5D4E3A):
    OX3A2B1C4D = OX6E88E3C6.arange(OX6E88E3C6.prod(OX3E4C5B2A), dtype=OX6E88E3C6.float32)
    OX3A2B1C4D = OX3A2B1C4D.reshape(OX3E4C5B2A)
    self.OX7A8C9B6D(OX3A2B1C4D, OX4E5F6A7B, OX6F5D4E3A,
                  OX1F9E7B4A(OX3A2B1C4D, OX4E5F6A7B, OX6F5D4E3A))

  def OX8C7D6E5F(self):
    self.OX7A8C9B6D(
        OX3A2B1C4D=[1, 2],
        OX4E5F6A7B=[],
        OX6F5D4E3A=[],
        OX5B3C2A1D=[1, 2],)

  def OX2A3B4C5D(self):
    self.OX7A8C9B6D(
        OX3A2B1C4D=[[1, 2], [3, 4]],
        OX4E5F6A7B=[],
        OX6F5D4E3A=[],
        OX5B3C2A1D=[[1, 2], [3, 4]])

    self.OX7A8C9B6D(
        OX3A2B1C4D=[[1, 2], [3, 4]],
        OX4E5F6A7B=[1],
        OX6F5D4E3A=[[0, 0]],
        OX5B3C2A1D=[[1, 2], [3, 4]])

  def OX1B2A3C4D(self):
    self.OX7A8C9B6D(
        OX3A2B1C4D=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]],
        OX4E5F6A7B=[],
        OX6F5D4E3A=[],
        OX5B3C2A1D=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]])

    self.OX7A8C9B6D(
        OX3A2B1C4D=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]],
        OX4E5F6A7B=[1],
        OX6F5D4E3A=[[0, 0]],
        OX5B3C2A1D=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]])

    self.OX7A8C9B6D(
        OX3A2B1C4D=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]],
        OX4E5F6A7B=[1, 1],
        OX6F5D4E3A=[[0, 0], [0, 0]],
        OX5B3C2A1D=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]])

  def OX3D2C1A4B(self):
    self.OX7A8C9B6D(
        OX3A2B1C4D=[[1, 2, 3], [4, 5, 6]],
        OX4E5F6A7B=[2],
        OX6F5D4E3A=[1, 0],
        OX5B3C2A1D=[[0, 2], [0, 5], [1, 3], [4, 6]])

  def OX4C5D6E7F(self):
    self.OX7A8C9B6D(
        OX3A2B1C4D=[[[1, 11], [2, 21], [3, 31]], [[4, 41], [5, 51], [6, 61]]],
        OX4E5F6A7B=[2],
        OX6F5D4E3A=[1, 0],
        OX5B3C2A1D=[[[0, 0], [2, 21]], [[0, 0], [5, 51]], [[1, 11], [3, 31]],
                 [[4, 41], [6, 61]]])

  def OX2D1F3C4B(self):
    self.OX3D2C1A4B(
        OX3E4C5B2A=[3, 1, 2, 0], OX4E5F6A7B=[3], OX6F5D4E3A=[[0, 2]])

  def OX1B2A3C4D(self):
    self.OX3D2C1A4B(
        OX3E4C5B2A=[3, 0, 2, 5], OX4E5F6A7B=[3], OX6F5D4E3A=[[0, 0]])

  def OX5D6E7F8A(self):
    self.OX3D2C1A4B(
        OX3E4C5B2A=[3, 0, 2, 5], OX4E5F6A7B=[3], OX6F5D4E3A=[[1, 2]])

  def OX9A8B7C6D(self):
    self.OX3D2C1A4B(
        OX3E4C5B2A=[3, 3, 4, 5, 2],
        OX4E5F6A7B=[3, 4, 2],
        OX6F5D4E3A=[[1, 2], [0, 0], [3, 0]])

  def OX7C8B9A6D(self):
    self.OX3D2C1A4B(
        OX3E4C5B2A=[3, 3, 4, 5, 2],
        OX4E5F6A7B=[3, 4, 2, 2],
        OX6F5D4E3A=[[1, 2], [0, 0], [3, 0], [0, 0]])

  def OX5C6B7A8D(self):
    self.OX3D2C1A4B(
        OX3E4C5B2A=[3, 2, 2, 3, 4, 5, 2, 5],
        OX4E5F6A7B=[1, 1, 3, 4, 2, 2],
        OX6F5D4E3A=[[0, 0], [0, 0], [1, 2], [0, 0], [3, 0], [0, 0]])

  def OX4A3B2C1D(self):
    self.OX3D2C1A4B(
        OX3E4C5B2A=[3, 2, 2, 3, 4, 5, 2, 5],
        OX4E5F6A7B=[1, 1, 3, 4, 2, 2, 1],
        OX6F5D4E3A=[[0, 0], [0, 0], [1, 2], [0, 0], [3, 0], [0, 0], [0, 0]])

if __name__ == "__main__":
  OX3B7F6D1E.main()