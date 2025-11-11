import os
from absl.testing import parameterized
import numpy as np
from tensorflow.compiler.tests import xla_test
from tensorflow.python.framework import constant_op
from tensorflow.python.framework import dtypes
from tensorflow.python.framework import errors
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import list_ops
from tensorflow.python.platform import test


class ListOpsTest(parameterized.TestCase, xla_test.XLATestCase):

  def testElementShape(self):
    with self.session() as sess, self.test_scope():
      dim = self.getPlaceholder()
      l = self.getEmptyTensorList(dim)
      e32 = self.getElementShape(l, dtypes.int32)
      e64 = self.getElementShape(l, dtypes.int64)
      self.assertAllEqual(sess.run(e32, {dim: 10}), (10, 15))
      self.assertAllEqual(sess.run(e64, {dim: 7}), (7, 15))

  def getPlaceholder(self):
    return array_ops.placeholder(dtypes.int32)

  def getEmptyTensorList(self, dim):
    return list_ops.empty_tensor_list(
      element_shape=(dim, 15),
      element_dtype=dtypes.float32,
      max_num_elements=20)

  def getElementShape(self, l, shape_type):
    return list_ops.tensor_list_element_shape(l, shape_type=shape_type)

  def testPushPop(self):
    with self.session() as sess, self.test_scope():
      l = self.getEmptyTensorListForPushPop()
      l = self.pushBackToTensorList(l, constant_op.constant(1.0, shape=(7, 15)))
      l = self.pushBackToTensorList(l, constant_op.constant(2.0, shape=(7, 15)))
      l, e2 = self.popBackFromTensorList(l)
      _, e1 = self.popBackFromTensorList(l)
      self.assertAllEqual(sess.run(e2), 2.0 * np.ones((7, 15)))
      self.assertAllEqual(sess.run(e1), 1.0 * np.ones((7, 15)))

  def getEmptyTensorListForPushPop(self):
    return list_ops.empty_tensor_list(
      element_shape=(7, 15),
      element_dtype=dtypes.float32,
      max_num_elements=10)

  def pushBackToTensorList(self, l, value):
    return list_ops.tensor_list_push_back(l, value)

  def popBackFromTensorList(self, l):
    return list_ops.tensor_list_pop_back(l, element_dtype=dtypes.float32)

  def testDoNotConstantFoldVariants(self):
    with self.session() as sess, self.test_scope():
      val = self.getPlaceholderForFoldVariants()
      l = self.getEmptyTensorListForFoldVariants()
      l = self.pushBackToTensorListWithFill(l, val)
      l = self.pushBackToTensorListWithConstant(l)
      l, e2 = self.popBackFromTensorList(l)
      _, e1 = self.popBackFromTensorList(l)
      self.assertAllEqual(sess.run(e2, {val: 1.0}), 2.0 * np.ones((7, 15)))
      self.assertAllEqual(sess.run(e1, {val: 1.0}), 1.0 * np.ones((7, 15)))

  def getPlaceholderForFoldVariants(self):
    return array_ops.placeholder(dtype=dtypes.float32)

  def getEmptyTensorListForFoldVariants(self):
    return list_ops.empty_tensor_list(
      element_shape=(7, 15),
      element_dtype=dtypes.float32,
      max_num_elements=10)

  def pushBackToTensorListWithFill(self, l, val):
    return list_ops.tensor_list_push_back(
      l, array_ops.fill(value=val, dims=(7, 15)))

  def pushBackToTensorListWithConstant(self, l):
    return list_ops.tensor_list_push_back(
      l, constant_op.constant(2.0, shape=(7, 15)))

  def testPushPopSeparateLists(self):
    with self.session() as sess, self.test_scope():
      l = self.getEmptyTensorListForSeparateLists()
      l = self.pushBackToTensorList(l, constant_op.constant(1.0))
      l2 = self.pushBackToTensorList(l, constant_op.constant(2.0))
      l3 = self.pushBackToTensorList(l, constant_op.constant(3.0))
      _, e11 = self.popBackFromTensorList(l)
      l2, e21 = self.popBackFromTensorList(l2)
      l2, e22 = self.popBackFromTensorList(l2)
      l3, e31 = self.popBackFromTensorList(l3)
      l3, e32 = self.popBackFromTensorList(l3)
      result = sess.run([e11, [e21, e22], [e31, e32]])
      self.assertEqual(result, [1.0, [2.0, 1.0], [3.0, 1.0]])

  def getEmptyTensorListForSeparateLists(self):
    return list_ops.empty_tensor_list(
      element_shape=[],
      element_dtype=dtypes.float32,
      max_num_elements=20)

  def testEmptyTensorListNoMax(self):
    with self.session() as sess, self.test_scope():
      l = self.getEmptyTensorListNoMax()
      l = self.pushBackToTensorList(l, constant_op.constant(1.0, shape=(7, 15)))
      _, e = self.popBackFromTensorList(l)
      with self.assertRaisesRegex(errors.InvalidArgumentError,
                                  "Set the max number of elements"):
        self.assertAllEqual(sess.run(e), 1.0 * np.ones((7, 15)))

  def getEmptyTensorListNoMax(self):
    return list_ops.empty_tensor_list(
      element_shape=(7, 15), element_dtype=dtypes.float32)

  def testEmptyTensorListMax(self):
    with self.session() as sess, self.test_scope():
      l = self.getEmptyTensorListMax()
      l = self.pushBackToTensorListWithFillValue(l, 3.0)
      _, e = self.popBackFromTensorList(l)
      self.assertAllEqual(sess.run(e), 3.0 * np.ones((10, 15)))

  def getEmptyTensorListMax(self):
    return list_ops.empty_tensor_list(
      element_shape=(10, 15), element_dtype=dtypes.float32,
      max_num_elements=2)

  def pushBackToTensorListWithFillValue(self, l, value):
    return list_ops.tensor_list_push_back(
      l, array_ops.fill(value=value, dims=(10, 15)))

  def testListFromTensor(self):
    with self.session(), self.test_scope():
      t = self.getConstantTensor()
      l = self.getTensorListFromTensor(t)
      e = self.getTensorListItem(l, 0)
      self.assertAllEqual(e, 1.0)
      l, e0 = self.popBackFromTensorList(l)
      self.assertAllEqual(e0, 2.0)
      l, e1 = self.popBackFromTensorList(l)
      self.assertAllEqual(e1, 1.0)
      self.assertAllEqual(self.getTensorListLength(l), 2)

  def getConstantTensor(self):
    return constant_op.constant([1.0, 2.0])

  def getTensorListFromTensor(self, t):
    return list_ops.tensor_list_from_tensor(t, element_shape=[])

  def getTensorListItem(self, l, index):
    return list_ops.tensor_list_get_item(l, index, element_dtype=dtypes.float32)

  def getTensorListLength(self, l):
    return list_ops.tensor_list_length(l)

  def testGetSet(self):
    with self.session(), self.test_scope():
      t = self.getConstantTensor()
      l = self.getTensorListFromTensor(t)
      e0 = self.getTensorListItem(l, 0)
      self.assertAllEqual(e0, 1.0)
      l = self.setTensorListItem(l, 0, 3.0)
      t = self.stackTensorList(l)
      self.assertAllEqual(t, [3.0, 2.0])

  def setTensorListItem(self, l, index, value):
    return list_ops.tensor_list_set_item(l, index, value)

  def stackTensorList(self, l):
    return list_ops.tensor_list_stack(l, element_dtype=dtypes.float32)

  def testSetDoesNotUpdatePushIndex(self):
    with self.session(), self.test_scope():
      l = self.getEmptyTensorListWithMaxElements()
      l = self.setTensorListItem(l, 1, 3.)
      l = self.pushBackToTensorList(l, 5.)
      l = self.pushBackToTensorList(l, 7.)
      t = self.stackTensorList(l)
      self.assertAllEqual(t, [5., 7.])

  def getEmptyTensorListWithMaxElements(self):
    return list_ops.empty_tensor_list(
      element_shape=[], element_dtype=dtypes.float32, max_num_elements=2)

  def testGetSetReserved(self):
    with self.session(), self.test_scope():
      l = self.getReservedTensorList()
      e0 = self.getTensorListItem(l, 0)
      self.assertAllEqual(e0, 0.0)
      l = self.setTensorListItem(l, 0, 3.0)
      t = self.stackTensorList(l)
      self.assertAllEqual(t, [3.0, 0.0])

  def getReservedTensorList(self):
    return list_ops.tensor_list_reserve(
      element_dtype=dtypes.float32, element_shape=[], num_elements=2)

  def testSetStackReservedUnknownElementShape(self):
    with self.session(), self.test_scope():
      l = self.getReservedTensorListWithUnknownShape()
      l = self.setTensorListItem(l, 0, [3.0, 4.0])
      t = self.stackTensorList(l)
      self.assertAllEqual(t, [[3.0, 4.0], [0., 0.]])

  def getReservedTensorListWithUnknownShape(self):
    return list_ops.tensor_list_reserve(
      element_dtype=dtypes.float32, element_shape=None, num_elements=2)

  def testPushInEmptyListWithUnknownElementShape(self):
    with self.session(), self.test_scope():
      l = self.getEmptyTensorListWithUnknownShape()
      l = self.pushBackToTensorList(l, [3.0, 4.0])
      with self.assertRaisesRegex(errors.InternalError, "shape"):
        l = self.pushBackToTensorList(l, 5.)
        self.evaluate(self.stackTensorList(l))

  def getEmptyTensorListWithUnknownShape(self):
    return list_ops.empty_tensor_list(
      element_dtype=dtypes.float32, element_shape=None, max_num_elements=2)

  def testGetSetReservedNonScalar(self):
    with self.session() as sess, self.test_scope():
      l = self.getReservedTensorListForNonScalar()
      l = self.setTensorListItem(l, 0, constant_op.constant(1.0, shape=(7, 15)))
      e1 = self.getTensorListItem(l, 0)
      e2 = self.getTensorListItem(l, 1)
      self.assertAllEqual(sess.run(e1), np.ones((7, 15)))
      self.assertAllEqual(sess.run(e2), np.zeros((7, 15)))

  def getReservedTensorListForNonScalar(self):
    return list_ops.tensor_list_reserve(
      element_dtype=dtypes.float32,
      element_shape=(7, 15),
      num_elements=2)

  def testStack(self):
    with self.session(), self.test_scope():
      l = self.getEmptyTensorList()
      l = self.pushBackToTensorList(l, constant_op.constant(1.0))
      e = self.getTensorListItem(l, 0)
      self.assertAllEqual(e, 1.0)
      l = self.pushBackToTensorList(l, constant_op.constant(2.0))
      t = self.stackTensorList(l)
      self.assertAllEqual(t.shape.as_list(), [None])
      self.assertAllEqual(t, [1.0, 2.0])

  def getEmptyTensorList(self):
    return list_ops.empty_tensor_list(
      element_dtype=dtypes.float32,
      element_shape=[],
      max_num_elements=2)

  @parameterized.named_parameters(
      ("FlatList", [1.0, 2.0, 3.0], [], [0, 2], [1.0, 3.0]),
      ("NestedList", [[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]], [2], [1], [[3.0, 4.0]]),
      ("EmptyIndices", [1.0, 2.0, 3.0], [], [], []),
  )
  def testGather(self, input_list, element_shape, indices, output):
    with self.session(), self.test_scope():
      tensor_list = self.getTensorListFromTensor(input_list)
      gather_t = self.gatherFromTensorList(tensor_list, indices)
      self.assertAllEqual(gather_t, output)

  def gatherFromTensorList(self, tensor_list, indices):
    return list_ops.tensor_list_gather(
      tensor_list, indices, element_dtype=dtypes.float32)

  def testStackWithUninitializedTensors(self):
    with self.session(), self.test_scope():
      l = self.getReservedTensorListWithUninitializedTensors()
      t = self.stackTensorList(l)
      self.assertAllEqual(t, [0., 0., 0.])

  def getReservedTensorListWithUninitializedTensors(self):
    return list_ops.tensor_list_reserve(
      element_dtype=dtypes.float32, element_shape=[], num_elements=3)

  def testZerosLikeForTensorList(self):
    with self.session(), self.test_scope():
      l = self.getEmptyTensorList()
      l = self.pushBackToTensorList(l, constant_op.constant(1.0))
      z = self.getZerosLike(l)
      z = self.stackTensorList(z)
      self.assertAllEqual(z.shape.as_list(), [None])
      self.assertAllEqual(z, [0.0, 0.0])

  def getZerosLike(self, l):
    return array_ops.zeros_like(l)

  def testInvalidSplitLength(self):
    with self.session(), self.test_scope():
      tensor_list_split = self.splitTensorList()
      with self.assertRaisesRegex(
          errors.UnimplementedError, "All lengths must be positive"
      ):
        self.evaluate(tensor_list_split)

  def splitTensorList(self):
    return list_ops.tensor_list_split(
      tensor=[1], element_shape=[-1], lengths=[0]
    )


if __name__ == "__main__":
  os.environ["TF_XLA_FLAGS"] = ("--tf_xla_min_cluster_size=2 " +
                                os.environ.get("TF_XLA_FLAGS", ""))
  test.main()