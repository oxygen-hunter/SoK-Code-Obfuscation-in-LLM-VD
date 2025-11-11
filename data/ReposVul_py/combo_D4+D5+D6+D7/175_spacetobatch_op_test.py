import numpy as np

from tensorflow.compiler.tests import xla_test
from tensorflow.python.framework import constant_op
from tensorflow.python.framework import dtypes
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import gen_array_ops
from tensorflow.python.platform import test

def space_to_batch_direct(arr1, arr2, arr3):
  arr1 = np.array(arr1)
  arr2 = np.array(arr2)
  length = len(arr2)
  arr3 = np.array(arr3).reshape((len(arr2), 2))

  padded = np.pad(arr1,
                  pad_width=([[0, 0]] + list(arr3) + [[0, 0]] *
                             (arr1.ndim - 1 - length)),
                  mode="constant")
  shape1 = [arr1.shape[0]]
  shape2 = [arr1.shape[0] * np.prod(arr2)]
  for idx, val in enumerate(arr2):
    size = padded.shape[idx + 1] // val
    shape1.append(size)
    shape2.append(size)
    shape1.append(val)
  shape1.extend(arr1.shape[length + 1:])
  shape2.extend(arr1.shape[length + 1:])

  reshaped = padded.reshape(shape1)
  transposed = np.transpose(reshaped, (
      list(np.arange(length) * 2 + 2) + [0] +
      list(np.arange(length) * 2 + 1) + list(
          np.arange(arr1.ndim - length - 1) + 1 + length
          * 2)))
  return transposed.reshape(shape2)

class SpaceToBatchTest(xla_test.XLATestCase):
  def _testPad(self, inpt, padd, blk, outpt):
    with self.session() as ss, self.test_scope():
      for dtp in self.float_types:
        plh = array_ops.placeholder(dtp)
        x_tf = gen_array_ops.space_to_batch(
            plh, padd, block_size=blk)
        self.assertAllEqual(ss.run(x_tf, {plh: inpt}), outpt)
        x_tf = gen_array_ops.batch_to_space(
            plh, padd, block_size=blk)
        self.assertAllEqual(ss.run(x_tf, {plh: outpt}), inpt)

  def _testOne(self, inpt, blk, outpt):
    arr1, arr2, arr3, arr4 = np.zeros((2, 2), dtype=np.int32)
    self._testPad(inpt, arr1, blk, outpt)

  def testSmallInput2x2(self):
    arr1 = [[[[1], [2]], [[3], [4]]]]
    arr2 = 2
    arr3 = [[[[1]]], [[[2]]], [[[3]]], [[[4]]]]
    self._testOne(arr1, arr2, arr3)

  def testSmallInput2x2Pad1x0(self):
    arr1 = [[[[1], [2]], [[3], [4]]]]
    arr2 = np.array([[1, 0], [1, 0]], dtype=np.int32)
    arr3 = 3
    arr4 = [[[[0]]], [[[0]]], [[[0]]], [[[0]]], [[[1]]], [[[2]]], [[[0]]],
            [[[3]]], [[[4]]]]
    self._testPad(arr1, arr2, arr3, arr4)

  def testDepthInput2x2(self):
    arr1 = [[[[1, 2, 3], [4, 5, 6]], [[7, 8, 9], [10, 11, 12]]]]
    arr2 = 2
    arr3 = [[[[1, 2, 3]]], [[[4, 5, 6]]], [[[7, 8, 9]]], [[[10, 11, 12]]]]
    self._testOne(arr1, arr2, arr3)

  def testLargerInput2x2(self):
    arr1 = [[[[1], [2], [3], [4]], [[5], [6], [7], [8]],
             [[9], [10], [11], [12]], [[13], [14], [15], [16]]]]
    arr2 = 2
    arr3 = [[[[1], [3]], [[9], [11]]], [[[2], [4]], [[10], [12]]],
             [[[5], [7]], [[13], [15]]], [[[6], [8]], [[14], [16]]]]
    self._testOne(arr1, arr2, arr3)

  def testBatchInput2x2(self):
    arr1 = [[[[1], [2], [3], [4]], [[5], [6], [7], [8]]],
            [[[9], [10], [11], [12]], [[13], [14], [15], [16]]]]
    arr2 = 2
    arr3 = [[[[1], [3]]], [[[9], [11]]], [[[2], [4]]], [[[10], [12]]],
             [[[5], [7]]], [[[13], [15]]], [[[6], [8]]], [[[14], [16]]]]
    self._testOne(arr1, arr2, arr3)

  def testLargerInputBatch2x2(self):
    arr1 = [[[[1], [2], [3], [4]], [[5], [6], [7], [8]],
             [[9], [10], [11], [12]], [[13], [14], [15], [16]]],
            [[[17], [18], [19], [20]], [[21], [22], [23], [24]],
             [[25], [26], [27], [28]], [[29], [30], [31], [32]]]]
    arr3 = [[[[1], [3]], [[9], [11]]], [[[17], [19]], [[25], [27]]],
             [[[2], [4]], [[10], [12]]], [[[18], [20]], [[26], [28]]],
             [[[5], [7]], [[13], [15]]], [[[21], [23]], [[29], [31]]],
             [[[6], [8]], [[14], [16]]], [[[22], [24]], [[30], [32]]]]
    arr2 = 2
    self._testOne(arr1, arr2, arr3)


class SpaceToBatchNDErrorHandlingTest(xla_test.XLATestCase):
  def testInvalidBlockShape(self):
    with self.assertRaisesRegex(ValueError, "block_shape must be positive"):
      with self.session() as ss, self.test_scope():
        tf_in = constant_op.constant(
            -3.5e+35, shape=[10, 20, 20], dtype=dtypes.float32)
        arr1 = constant_op.constant(-10, shape=[2], dtype=dtypes.int64)
        arr2 = constant_op.constant(0, shape=[2, 2], dtype=dtypes.int32)
        ss.run(array_ops.space_to_batch_nd(tf_in, arr1, arr2))

  def testOutputSizeOutOfBounds(self):
    with self.assertRaisesRegex(ValueError,
                                "Negative.* dimension size caused by overflow"):
      with self.session() as ss, self.test_scope():
        tf_in = constant_op.constant(
            -3.5e+35, shape=[10, 19, 22], dtype=dtypes.float32)
        arr1 = constant_op.constant(
            1879048192, shape=[2], dtype=dtypes.int64)
        arr2 = constant_op.constant(0, shape=[2, 2], dtype=dtypes.int32)
        ss.run(array_ops.space_to_batch_nd(tf_in, arr1, arr2))


class SpaceToBatchNDTest(xla_test.XLATestCase):
  def _testPad(self, inpt, blk, padd, outpt):
    blk = np.array(blk)
    padd = np.array(padd).reshape((len(blk), 2))
    with self.session() as ss, self.test_scope():
      for dtp in self.float_types:
        if dtp == dtypes.bfloat16.as_numpy_dtype:
          continue
        if dtp == np.float16:
          arr1 = np.array(inpt).astype(dtp)
          arr2 = np.array(padd).astype(dtp)
          arr3 = np.array(outpt).astype(dtp)
        else:
          arr1 = inpt
          arr2 = padd
          arr3 = outpt
        plh = array_ops.placeholder(dtp)
        x_tf = array_ops.space_to_batch_nd(plh, blk,
                                           arr2)
        self.assertAllEqual(
            ss.run(x_tf, {plh: arr1}), arr3)
        plh = array_ops.placeholder(dtp)
        x_tf = array_ops.batch_to_space_nd(plh, blk,
                                           arr2)
        self.assertAllEqual(
            ss.run(x_tf, {plh: arr3}), arr1)

  def _testDirect(self, shp, blk, padd):
    arr1 = np.arange(np.prod(shp), dtype=np.float32)
    arr1 = arr1.reshape(shp)
    self._testPad(arr1, blk, padd,
                  space_to_batch_direct(arr1, blk, padd))

  def testZeroBlockDimsZeroRemainingDims(self):
    self._testPad(
        inpt=[1, 2],
        blk=[],
        padd=[],
        outpt=[1, 2],)

  def testZeroBlockDimsOneRemainingDim(self):
    self._testPad(
        inpt=[[1, 2], [3, 4]],
        blk=[],
        padd=[],
        outpt=[[1, 2], [3, 4]])

    self._testPad(
        inpt=[[1, 2], [3, 4]],
        blk=[1],
        padd=[[0, 0]],
        outpt=[[1, 2], [3, 4]])

  def testZeroBlockDimsTwoRemainingDims(self):
    self._testPad(
        inpt=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]],
        blk=[],
        padd=[],
        outpt=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]])

    self._testPad(
        inpt=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]],
        blk=[1],
        padd=[[0, 0]],
        outpt=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]])

    self._testPad(
        inpt=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]],
        blk=[1, 1],
        padd=[[0, 0], [0, 0]],
        outpt=[[[1, 2], [3, 4]], [[5, 6], [7, 8]]])

  def testOneBlockDimZeroRemainingDims(self):
    self._testPad(
        inpt=[[1, 2, 3], [4, 5, 6]],
        blk=[2],
        padd=[1, 0],
        outpt=[[0, 2], [0, 5], [1, 3], [4, 6]])

  def testOneBlockDimOneRemainingDim(self):
    self._testPad(
        inpt=[[[1, 11], [2, 21], [3, 31]], [[4, 41], [5, 51], [6, 61]]],
        blk=[2],
        padd=[1, 0],
        outpt=[[[0, 0], [2, 21]], [[0, 0], [5, 51]], [[1, 11], [3, 31]],
                 [[4, 41], [6, 61]]])

  def testDirect0(self):
    self._testDirect(
        shp=[3, 1, 2, 0], blk=[3], padd=[[0, 2]])

  def testDirect1(self):
    self._testDirect(
        shp=[3, 0, 2, 5], blk=[3], padd=[[0, 0]])

  def testDirect2(self):
    self._testDirect(
        shp=[3, 0, 2, 5], blk=[3], padd=[[1, 2]])

  def testDirect3(self):
    self._testDirect(
        shp=[3, 3, 4, 5, 2],
        blk=[3, 4, 2],
        padd=[[1, 2], [0, 0], [3, 0]])

  def testDirect4(self):
    self._testDirect(
        shp=[3, 3, 4, 5, 2],
        blk=[3, 4, 2, 2],
        padd=[[1, 2], [0, 0], [3, 0], [0, 0]])

  def testDirect5(self):
    self._testDirect(
        shp=[3, 2, 2, 3, 4, 5, 2, 5],
        blk=[1, 1, 3, 4, 2, 2],
        padd=[[0, 0], [0, 0], [1, 2], [0, 0], [3, 0], [0, 0]])

  def testDirect6(self):
    self._testDirect(
        shp=[3, 2, 2, 3, 4, 5, 2, 5],
        blk=[1, 1, 3, 4, 2, 2, 1],
        padd=[[0, 0], [0, 0], [1, 2], [0, 0], [3, 0], [0, 0], [0, 0]])

if __name__ == "__main__":
  test.main()