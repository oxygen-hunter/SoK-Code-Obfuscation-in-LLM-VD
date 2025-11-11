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


class OX7B4DF339(parameterized.TestCase, xla_test.XLATestCase):

  def OX5A6C2E28(self):
    with self.session() as OX2F4D5B53, self.test_scope():
      OX7F3A1B6E = array_ops.placeholder(dtypes.int32)
      OX5D9B1E55 = list_ops.empty_tensor_list(
          element_shape=(OX7F3A1B6E, 15),
          element_dtype=dtypes.float32,
          max_num_elements=20)
      OX1A8E4F65 = list_ops.tensor_list_element_shape(OX5D9B1E55, shape_type=dtypes.int32)
      OX3D9F2A47 = list_ops.tensor_list_element_shape(OX5D9B1E55, shape_type=dtypes.int64)
      self.assertAllEqual(OX2F4D5B53.run(OX1A8E4F65, {OX7F3A1B6E: 10}), (10, 15))
      self.assertAllEqual(OX2F4D5B53.run(OX3D9F2A47, {OX7F3A1B6E: 7}), (7, 15))

  def OX6C7D3E21(self):
    with self.session() as OX2F4D5B53, self.test_scope():
      OX5D9B1E55 = list_ops.empty_tensor_list(
          element_shape=(7, 15),
          element_dtype=dtypes.float32,
          max_num_elements=10)
      OX5D9B1E55 = list_ops.tensor_list_push_back(
          OX5D9B1E55, constant_op.constant(1.0, shape=(7, 15)))
      OX5D9B1E55 = list_ops.tensor_list_push_back(
          OX5D9B1E55, constant_op.constant(2.0, shape=(7, 15)))
      OX5D9B1E55, OX3E4A2C17 = list_ops.tensor_list_pop_back(OX5D9B1E55, element_dtype=dtypes.float32)
      _, OX1F5E3B29 = list_ops.tensor_list_pop_back(OX5D9B1E55, element_dtype=dtypes.float32)
      self.assertAllEqual(OX2F4D5B53.run(OX3E4A2C17), 2.0 * np.ones((7, 15)))
      self.assertAllEqual(OX2F4D5B53.run(OX1F5E3B29), 1.0 * np.ones((7, 15)))

  def OX2B3E5F48(self):
    with self.session() as OX2F4D5B53, self.test_scope():
      OX6A2D4C19 = array_ops.placeholder(dtype=dtypes.float32)
      OX5D9B1E55 = list_ops.empty_tensor_list(
          element_shape=(7, 15),
          element_dtype=dtypes.float32,
          max_num_elements=10)
      OX5D9B1E55 = list_ops.tensor_list_push_back(
          OX5D9B1E55, array_ops.fill(value=OX6A2D4C19, dims=(7, 15)))
      OX5D9B1E55 = list_ops.tensor_list_push_back(
          OX5D9B1E55, constant_op.constant(2.0, shape=(7, 15)))
      OX5D9B1E55, OX3E4A2C17 = list_ops.tensor_list_pop_back(OX5D9B1E55, element_dtype=dtypes.float32)
      _, OX1F5E3B29 = list_ops.tensor_list_pop_back(OX5D9B1E55, element_dtype=dtypes.float32)
      self.assertAllEqual(OX2F4D5B53.run(OX3E4A2C17, {OX6A2D4C19: 1.0}), 2.0 * np.ones((7, 15)))
      self.assertAllEqual(OX2F4D5B53.run(OX1F5E3B29, {OX6A2D4C19: 1.0}), 1.0 * np.ones((7, 15)))

  def OX4D2F1B87(self):
    with self.session() as OX2F4D5B53, self.test_scope():
      OX5D9B1E55 = list_ops.empty_tensor_list(
          element_shape=[],
          element_dtype=dtypes.float32,
          max_num_elements=20)
      OX5D9B1E55 = list_ops.tensor_list_push_back(OX5D9B1E55, constant_op.constant(1.0))
      OX1D4C7E35 = list_ops.tensor_list_push_back(OX5D9B1E55, constant_op.constant(2.0))
      OX1E3B7D49 = list_ops.tensor_list_push_back(OX5D9B1E55, constant_op.constant(3.0))
      _, OX7E3A5C17 = list_ops.tensor_list_pop_back(OX5D9B1E55, element_dtype=dtypes.float32)
      OX1D4C7E35, OX6F2B3E18 = list_ops.tensor_list_pop_back(OX1D4C7E35, element_dtype=dtypes.float32)
      OX1D4C7E35, OX8C3F1B27 = list_ops.tensor_list_pop_back(OX1D4C7E35, element_dtype=dtypes.float32)
      OX1E3B7D49, OX9E4A2C29 = list_ops.tensor_list_pop_back(OX1E3B7D49, element_dtype=dtypes.float32)
      OX1E3B7D49, OX2F5D4E32 = list_ops.tensor_list_pop_back(OX1E3B7D49, element_dtype=dtypes.float32)
      OX5C3D6F48 = OX2F4D5B53.run([OX7E3A5C17, [OX6F2B3E18, OX8C3F1B27], [OX9E4A2C29, OX2F5D4E32]])
      self.assertEqual(OX5C3D6F48, [1.0, [2.0, 1.0], [3.0, 1.0]])

  def OX5F4A3C98(self):
    with self.session() as OX2F4D5B53, self.test_scope():
      OX5D9B1E55 = list_ops.empty_tensor_list(
          element_shape=(7, 15), element_dtype=dtypes.float32)
      OX5D9B1E55 = list_ops.tensor_list_push_back(
          OX5D9B1E55, constant_op.constant(1.0, shape=(7, 15)))
      _, OX2C7B5E16 = list_ops.tensor_list_pop_back(OX5D9B1E55, element_dtype=dtypes.float32)
      with self.assertRaisesRegex(errors.InvalidArgumentError,
                                  "Set the max number of elements"):
        self.assertAllEqual(OX2F4D5B53.run(OX2C7B5E16), 1.0 * np.ones((7, 15)))

  def OX7D2E3C99(self):
    with self.session() as OX2F4D5B53, self.test_scope():
      OX5D9B1E55 = list_ops.empty_tensor_list(
          element_shape=(10, 15), element_dtype=dtypes.float32,
          max_num_elements=2)
      OX5D9B1E55 = list_ops.tensor_list_push_back(
          OX5D9B1E55, array_ops.fill(value=3.0, dims=(10, 15)))
      _, OX4E3B2D26 = list_ops.tensor_list_pop_back(OX5D9B1E55, element_dtype=dtypes.float32)
      self.assertAllEqual(OX2F4D5B53.run(OX4E3B2D26), 3.0 * np.ones((10, 15)))

  def OX1A3C5E77(self):
    with self.session(), self.test_scope():
      OX4A5B3C27 = constant_op.constant([1.0, 2.0])
      OX5D9B1E55 = list_ops.tensor_list_from_tensor(OX4A5B3C27, element_shape=[])
      OX3D6B4F37 = list_ops.tensor_list_get_item(OX5D9B1E55, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(OX3D6B4F37, 1.0)
      OX5D9B1E55, OX2C7A5D17 = list_ops.tensor_list_pop_back(OX5D9B1E55, element_dtype=dtypes.float32)
      self.assertAllEqual(OX2C7A5D17, 2.0)
      OX5D9B1E55, OX8F2A3B19 = list_ops.tensor_list_pop_back(OX5D9B1E55, element_dtype=dtypes.float32)
      self.assertAllEqual(OX8F2A3B19, 1.0)
      self.assertAllEqual(list_ops.tensor_list_length(OX5D9B1E55), 2)

  def OX7A4F2E68(self):
    with self.session(), self.test_scope():
      OX4A5B3C27 = constant_op.constant([1.0, 2.0])
      OX5D9B1E55 = list_ops.tensor_list_from_tensor(OX4A5B3C27, element_shape=[])
      OX9D3B5E27 = list_ops.tensor_list_get_item(OX5D9B1E55, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(OX9D3B5E27, 1.0)
      OX5D9B1E55 = list_ops.tensor_list_set_item(OX5D9B1E55, 0, 3.0)
      OX4E3B2D26 = list_ops.tensor_list_stack(OX5D9B1E55, element_dtype=dtypes.float32)
      self.assertAllEqual(OX4E3B2D26, [3.0, 2.0])

  def OX2C5B3A87(self):
    with self.session(), self.test_scope():
      OX5D9B1E55 = list_ops.empty_tensor_list(
          element_shape=[], element_dtype=dtypes.float32, max_num_elements=2)
      OX5D9B1E55 = list_ops.tensor_list_set_item(OX5D9B1E55, 1, 3.)
      OX5D9B1E55 = list_ops.tensor_list_push_back(OX5D9B1E55, 5.)
      OX5D9B1E55 = list_ops.tensor_list_push_back(OX5D9B1E55, 7.)
      OX4E3B2D26 = list_ops.tensor_list_stack(OX5D9B1E55, element_dtype=dtypes.float32)
      self.assertAllEqual(OX4E3B2D26, [5., 7.])

  def OX5C4A3F58(self):
    with self.session(), self.test_scope():
      OX5D9B1E55 = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32, element_shape=[], num_elements=2)
      OX9D3B5E27 = list_ops.tensor_list_get_item(OX5D9B1E55, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(OX9D3B5E27, 0.0)
      OX5D9B1E55 = list_ops.tensor_list_set_item(OX5D9B1E55, 0, 3.0)
      OX4E3B2D26 = list_ops.tensor_list_stack(OX5D9B1E55, element_dtype=dtypes.float32)
      self.assertAllEqual(OX4E3B2D26, [3.0, 0.0])

  def OX3F2B5A69(self):
    with self.session(), self.test_scope():
      OX5D9B1E55 = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32, element_shape=None, num_elements=2)
      OX5D9B1E55 = list_ops.tensor_list_set_item(OX5D9B1E55, 0, [3.0, 4.0])
      OX4E3B2D26 = list_ops.tensor_list_stack(OX5D9B1E55, element_dtype=dtypes.float32)
      self.assertAllEqual(OX4E3B2D26, [[3.0, 4.0], [0., 0.]])

  def OX2A5C3D47(self):
    with self.session(), self.test_scope():
      OX5D9B1E55 = list_ops.empty_tensor_list(
          element_dtype=dtypes.float32, element_shape=None, max_num_elements=2)
      OX5D9B1E55 = list_ops.tensor_list_push_back(OX5D9B1E55, [3.0, 4.0])
      with self.assertRaisesRegex(errors.InternalError, "shape"):
        OX5D9B1E55 = list_ops.tensor_list_push_back(OX5D9B1E55, 5.)
        self.evaluate(
            list_ops.tensor_list_stack(OX5D9B1E55, element_dtype=dtypes.float32))

  def OX4B3C5E98(self):
    with self.session() as OX2F4D5B53, self.test_scope():
      OX5D9B1E55 = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32,
          element_shape=(7, 15),
          num_elements=2)
      OX5D9B1E55 = list_ops.tensor_list_set_item(
          OX5D9B1E55, 0, constant_op.constant(1.0, shape=(7, 15)))
      OX9E3A5B17 = list_ops.tensor_list_get_item(OX5D9B1E55, 0, element_dtype=dtypes.float32)
      OX2A5D4C38 = list_ops.tensor_list_get_item(OX5D9B1E55, 1, element_dtype=dtypes.float32)
      self.assertAllEqual(OX2F4D5B53.run(OX9E3A5B17), np.ones((7, 15)))
      self.assertAllEqual(OX2F4D5B53.run(OX2A5D4C38), np.zeros((7, 15)))

  def OX6D2F4A77(self):
    with self.session(), self.test_scope():
      OX5D9B1E55 = list_ops.empty_tensor_list(
          element_dtype=dtypes.float32,
          element_shape=[],
          max_num_elements=2)
      OX5D9B1E55 = list_ops.tensor_list_push_back(OX5D9B1E55, constant_op.constant(1.0))
      OX9D3B5E27 = list_ops.tensor_list_get_item(OX5D9B1E55, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(OX9D3B5E27, 1.0)
      OX5D9B1E55 = list_ops.tensor_list_push_back(OX5D9B1E55, constant_op.constant(2.0))
      OX4E3B2D26 = list_ops.tensor_list_stack(OX5D9B1E55, element_dtype=dtypes.float32)
      self.assertAllEqual(OX4E3B2D26.shape.as_list(), [None])
      self.assertAllEqual(OX4E3B2D26, [1.0, 2.0])

  @parameterized.named_parameters(
      ("OX3F1B2A77", [1.0, 2.0, 3.0], [], [0, 2], [1.0, 3.0]),
      ("OX4C3E5D99", [[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]
                     ], [2], [1], [[3.0, 4.0]]),
      ("OX6A2B5D57", [1.0, 2.0, 3.0], [], [], []),
  )
  def OX3B4D2F78(self, OX5B3C6D29, OX4C5A2E37, OX2E3F1A88, OX9D2B5C67):
    with self.session(), self.test_scope():
      OX5D9B1E55 = list_ops.tensor_list_from_tensor(
          OX5B3C6D29, element_shape=OX4C5A2E37)
      OX3D6B4F37 = list_ops.tensor_list_gather(
          OX5D9B1E55, OX2E3F1A88, element_dtype=dtypes.float32)
      self.assertAllEqual(OX3D6B4F37, OX9D2B5C67)

  def OX2F5C3A67(self):
    with self.session(), self.test_scope():
      OX5D9B1E55 = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32, element_shape=[], num_elements=3)
      OX4E3B2D26 = list_ops.tensor_list_stack(OX5D9B1E55, element_dtype=dtypes.float32)
      self.assertAllEqual(OX4E3B2D26, [0., 0., 0.])

  def OX6A3E5D98(self):
    with self.session(), self.test_scope():
      OX5D9B1E55 = list_ops.empty_tensor_list(
          element_dtype=dtypes.float32,
          element_shape=[],
          max_num_elements=2)
      OX5D9B1E55 = list_ops.tensor_list_push_back(OX5D9B1E55, constant_op.constant(1.0))
      OX3C5D4E78 = array_ops.zeros_like(OX5D9B1E55)
      OX3C5D4E78 = list_ops.tensor_list_stack(OX3C5D4E78, element_dtype=dtypes.float32)
      self.assertAllEqual(OX3C5D4E78.shape.as_list(), [None])
      self.assertAllEqual(OX3C5D4E78, [0.0, 0.0])

  def OX8C3D5B48(self):
    with self.session(), self.test_scope():
      OX1E3B5D29 = list_ops.tensor_list_split(
          tensor=[1], element_shape=[-1], lengths=[0]
      )
      with self.assertRaisesRegex(
          errors.UnimplementedError, "All lengths must be positive"
      ):
        self.evaluate(OX1E3B5D29)


if __name__ == "__main__":
  os.environ["TF_XLA_FLAGS"] = ("--tf_xla_min_cluster_size=2 " +
                                os.environ.get("TF_XLA_FLAGS", ""))
  test.main()