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
    with self.session() as a, self.test_scope():
      b, c = array_ops.placeholder(dtypes.int32), list_ops.empty_tensor_list(
          element_shape=(b, 15),
          element_dtype=dtypes.float32,
          max_num_elements=20)
      e32, e64 = list_ops.tensor_list_element_shape(c, shape_type=dtypes.int32), list_ops.tensor_list_element_shape(c, shape_type=dtypes.int64)
      self.assertAllEqual(a.run(e32, {b: 10}), (10, 15))
      self.assertAllEqual(a.run(e64, {b: 7}), (7, 15))

  def testPushPop(self):
    with self.session() as x, self.test_scope():
      y = list_ops.empty_tensor_list(
          element_shape=(7, 15),
          element_dtype=dtypes.float32,
          max_num_elements=10)
      y = list_ops.tensor_list_push_back(
          y, constant_op.constant(1.0, shape=(7, 15)))
      y = list_ops.tensor_list_push_back(
          y, constant_op.constant(2.0, shape=(7, 15)))
      y, z = list_ops.tensor_list_pop_back(y, element_dtype=dtypes.float32)
      _, w = list_ops.tensor_list_pop_back(y, element_dtype=dtypes.float32)
      self.assertAllEqual(x.run(z), 2.0 * np.ones((7, 15)))
      self.assertAllEqual(x.run(w), 1.0 * np.ones((7, 15)))

  def testDoNotConstantFoldVariants(self):
    with self.session() as u, self.test_scope():
      v = array_ops.placeholder(dtype=dtypes.float32)
      w = list_ops.empty_tensor_list(
          element_shape=(7, 15),
          element_dtype=dtypes.float32,
          max_num_elements=10)
      w = list_ops.tensor_list_push_back(
          w, array_ops.fill(value=v, dims=(7, 15)))
      w = list_ops.tensor_list_push_back(
          w, constant_op.constant(2.0, shape=(7, 15)))
      w, x = list_ops.tensor_list_pop_back(w, element_dtype=dtypes.float32)
      _, y = list_ops.tensor_list_pop_back(w, element_dtype=dtypes.float32)
      self.assertAllEqual(u.run(x, {v: 1.0}), 2.0 * np.ones((7, 15)))
      self.assertAllEqual(u.run(y, {v: 1.0}), 1.0 * np.ones((7, 15)))

  def testPushPopSeparateLists(self):
    with self.session() as q, self.test_scope():
      r = list_ops.empty_tensor_list(
          element_shape=[],
          element_dtype=dtypes.float32,
          max_num_elements=20)
      r = list_ops.tensor_list_push_back(r, constant_op.constant(1.0))
      s = list_ops.tensor_list_push_back(r, constant_op.constant(2.0))
      t = list_ops.tensor_list_push_back(r, constant_op.constant(3.0))
      _, u = list_ops.tensor_list_pop_back(r, element_dtype=dtypes.float32)
      s, v = list_ops.tensor_list_pop_back(s, element_dtype=dtypes.float32)
      s, w = list_ops.tensor_list_pop_back(s, element_dtype=dtypes.float32)
      t, x = list_ops.tensor_list_pop_back(t, element_dtype=dtypes.float32)
      t, y = list_ops.tensor_list_pop_back(t, element_dtype=dtypes.float32)
      z = q.run([u, [v, w], [x, y]])
      self.assertEqual(z, [1.0, [2.0, 1.0], [3.0, 1.0]])

  def testEmptyTensorListNoMax(self):
    with self.session() as i, self.test_scope():
      j = list_ops.empty_tensor_list(
          element_shape=(7, 15), element_dtype=dtypes.float32)
      j = list_ops.tensor_list_push_back(
          j, constant_op.constant(1.0, shape=(7, 15)))
      _, k = list_ops.tensor_list_pop_back(j, element_dtype=dtypes.float32)
      with self.assertRaisesRegex(errors.InvalidArgumentError,
                                  "Set the max number of elements"):
        self.assertAllEqual(i.run(k), 1.0 * np.ones((7, 15)))

  def testEmptyTensorListMax(self):
    with self.session() as m, self.test_scope():
      n = list_ops.empty_tensor_list(
          element_shape=(10, 15), element_dtype=dtypes.float32,
          max_num_elements=2)
      n = list_ops.tensor_list_push_back(
          n, array_ops.fill(value=3.0, dims=(10, 15)))
      _, o = list_ops.tensor_list_pop_back(n, element_dtype=dtypes.float32)
      self.assertAllEqual(m.run(o), 3.0 * np.ones((10, 15)))

  def testListFromTensor(self):
    with self.session(), self.test_scope():
      p = constant_op.constant([1.0, 2.0])
      q = list_ops.tensor_list_from_tensor(p, element_shape=[])
      r = list_ops.tensor_list_get_item(q, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(r, 1.0)
      q, s = list_ops.tensor_list_pop_back(q, element_dtype=dtypes.float32)
      self.assertAllEqual(s, 2.0)
      q, t = list_ops.tensor_list_pop_back(q, element_dtype=dtypes.float32)
      self.assertAllEqual(t, 1.0)
      self.assertAllEqual(list_ops.tensor_list_length(q), 2)

  def testGetSet(self):
    with self.session(), self.test_scope():
      a = constant_op.constant([1.0, 2.0])
      b = list_ops.tensor_list_from_tensor(a, element_shape=[])
      c = list_ops.tensor_list_get_item(b, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(c, 1.0)
      b = list_ops.tensor_list_set_item(b, 0, 3.0)
      a = list_ops.tensor_list_stack(b, element_dtype=dtypes.float32)
      self.assertAllEqual(a, [3.0, 2.0])

  def testSetDoesNotUpdatePushIndex(self):
    with self.session(), self.test_scope():
      g = list_ops.empty_tensor_list(
          element_shape=[], element_dtype=dtypes.float32, max_num_elements=2)
      g = list_ops.tensor_list_set_item(g, 1, 3.)
      g = list_ops.tensor_list_push_back(g, 5.)
      g = list_ops.tensor_list_push_back(g, 7.)
      h = list_ops.tensor_list_stack(g, element_dtype=dtypes.float32)
      self.assertAllEqual(h, [5., 7.])

  def testGetSetReserved(self):
    with self.session(), self.test_scope():
      u = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32, element_shape=[], num_elements=2)
      v = list_ops.tensor_list_get_item(u, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(v, 0.0)
      u = list_ops.tensor_list_set_item(u, 0, 3.0)
      w = list_ops.tensor_list_stack(u, element_dtype=dtypes.float32)
      self.assertAllEqual(w, [3.0, 0.0])

  def testSetStackReservedUnknownElementShape(self):
    with self.session(), self.test_scope():
      x = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32, element_shape=None, num_elements=2)
      x = list_ops.tensor_list_set_item(x, 0, [3.0, 4.0])
      y = list_ops.tensor_list_stack(x, element_dtype=dtypes.float32)
      self.assertAllEqual(y, [[3.0, 4.0], [0., 0.]])

  def testPushInEmptyListWithUnknownElementShape(self):
    with self.session(), self.test_scope():
      z = list_ops.empty_tensor_list(
          element_dtype=dtypes.float32, element_shape=None, max_num_elements=2)
      z = list_ops.tensor_list_push_back(z, [3.0, 4.0])
      with self.assertRaisesRegex(errors.InternalError, "shape"):
        z = list_ops.tensor_list_push_back(z, 5.)
        self.evaluate(
            list_ops.tensor_list_stack(z, element_dtype=dtypes.float32))

  def testGetSetReservedNonScalar(self):
    with self.session() as a, self.test_scope():
      b = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32,
          element_shape=(7, 15),
          num_elements=2)
      b = list_ops.tensor_list_set_item(
          b, 0, constant_op.constant(1.0, shape=(7, 15)))
      c = list_ops.tensor_list_get_item(b, 0, element_dtype=dtypes.float32)
      d = list_ops.tensor_list_get_item(b, 1, element_dtype=dtypes.float32)
      self.assertAllEqual(a.run(c), np.ones((7, 15)))
      self.assertAllEqual(a.run(d), np.zeros((7, 15)))

  def testStack(self):
    with self.session(), self.test_scope():
      e = list_ops.empty_tensor_list(
          element_dtype=dtypes.float32,
          element_shape=[],
          max_num_elements=2)
      e = list_ops.tensor_list_push_back(e, constant_op.constant(1.0))
      f = list_ops.tensor_list_get_item(e, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(f, 1.0)
      e = list_ops.tensor_list_push_back(e, constant_op.constant(2.0))
      g = list_ops.tensor_list_stack(e, element_dtype=dtypes.float32)
      self.assertAllEqual(g.shape.as_list(), [None])
      self.assertAllEqual(g, [1.0, 2.0])

  @parameterized.named_parameters(
      ("FlatList", [1.0, 2.0, 3.0], [], [0, 2], [1.0, 3.0]),
      ("NestedList", [[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]
                     ], [2], [1], [[3.0, 4.0]]),
      ("EmptyIndices", [1.0, 2.0, 3.0], [], [], []),
  )
  def testGather(self, input_list, element_shape, indices, output):
    with self.session(), self.test_scope():
      h = list_ops.tensor_list_from_tensor(
          input_list, element_shape=element_shape)
      i = list_ops.tensor_list_gather(
          h, indices, element_dtype=dtypes.float32)
      self.assertAllEqual(i, output)

  def testStackWithUninitializedTensors(self):
    with self.session(), self.test_scope():
      j = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32, element_shape=[], num_elements=3)
      k = list_ops.tensor_list_stack(j, element_dtype=dtypes.float32)
      self.assertAllEqual(k, [0., 0., 0.])

  def testZerosLikeForTensorList(self):
    with self.session(), self.test_scope():
      l = list_ops.empty_tensor_list(
          element_dtype=dtypes.float32,
          element_shape=[],
          max_num_elements=2)
      l = list_ops.tensor_list_push_back(l, constant_op.constant(1.0))
      m = array_ops.zeros_like(l)
      m = list_ops.tensor_list_stack(m, element_dtype=dtypes.float32)
      self.assertAllEqual(m.shape.as_list(), [None])
      self.assertAllEqual(m, [0.0, 0.0])

  def testInvalidSplitLength(self):
    with self.session(), self.test_scope():
      n = list_ops.tensor_list_split(
          tensor=[1], element_shape=[-1], lengths=[0]
      )
      with self.assertRaisesRegex(
          errors.UnimplementedError, "All lengths must be positive"
      ):
        self.evaluate(n)

if __name__ == "__main__":
  os.environ["TF_XLA_FLAGS"] = ("--tf_xla_min_cluster_size=2 " +
                                os.environ.get("TF_XLA_FLAGS", ""))
  test.main()