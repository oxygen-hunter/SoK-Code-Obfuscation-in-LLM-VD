# Copyright 2018 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
"""Tests for ops which manipulate lists of tensors via bridge."""

# pylint: disable=g-bad-name
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

  def OX5F8B9A72(self):
    with self.session() as OX9C2A1D71, self.test_scope():
      OXEE3A9C3D = array_ops.placeholder(dtypes.int32)
      OX0F66D7E4 = list_ops.empty_tensor_list(
          element_shape=(OXEE3A9C3D, 15),
          element_dtype=dtypes.float32,
          max_num_elements=20)
      OXE0A7F6FC = list_ops.tensor_list_element_shape(OX0F66D7E4, shape_type=dtypes.int32)
      OX4708B4EE = list_ops.tensor_list_element_shape(OX0F66D7E4, shape_type=dtypes.int64)
      self.assertAllEqual(OX9C2A1D71.run(OXE0A7F6FC, {OXEE3A9C3D: 10}), (10, 15))
      self.assertAllEqual(OX9C2A1D71.run(OX4708B4EE, {OXEE3A9C3D: 7}), (7, 15))

  def OX9F4C1DAB(self):
    with self.session() as OX9C2A1D71, self.test_scope():
      OX0F66D7E4 = list_ops.empty_tensor_list(
          element_shape=(7, 15),
          element_dtype=dtypes.float32,
          max_num_elements=10)
      OX0F66D7E4 = list_ops.tensor_list_push_back(
          OX0F66D7E4, constant_op.constant(1.0, shape=(7, 15)))
      OX0F66D7E4 = list_ops.tensor_list_push_back(
          OX0F66D7E4, constant_op.constant(2.0, shape=(7, 15)))
      OX0F66D7E4, OX0C8D9E7F = list_ops.tensor_list_pop_back(OX0F66D7E4, element_dtype=dtypes.float32)
      _, OX98FA1E6F = list_ops.tensor_list_pop_back(OX0F66D7E4, element_dtype=dtypes.float32)
      self.assertAllEqual(OX9C2A1D71.run(OX0C8D9E7F), 2.0 * np.ones((7, 15)))
      self.assertAllEqual(OX9C2A1D71.run(OX98FA1E6F), 1.0 * np.ones((7, 15)))

  def OX6F2E1A7D(self):
    with self.session() as OX9C2A1D71, self.test_scope():
      OXAA2E0B21 = array_ops.placeholder(dtype=dtypes.float32)
      OX0F66D7E4 = list_ops.empty_tensor_list(
          element_shape=(7, 15),
          element_dtype=dtypes.float32,
          max_num_elements=10)
      OX0F66D7E4 = list_ops.tensor_list_push_back(
          OX0F66D7E4, array_ops.fill(value=OXAA2E0B21, dims=(7, 15)))
      OX0F66D7E4 = list_ops.tensor_list_push_back(
          OX0F66D7E4, constant_op.constant(2.0, shape=(7, 15)))
      OX0F66D7E4, OX0C8D9E7F = list_ops.tensor_list_pop_back(OX0F66D7E4, element_dtype=dtypes.float32)
      _, OX98FA1E6F = list_ops.tensor_list_pop_back(OX0F66D7E4, element_dtype=dtypes.float32)
      self.assertAllEqual(OX9C2A1D71.run(OX0C8D9E7F, {OXAA2E0B21: 1.0}), 2.0 * np.ones((7, 15)))
      self.assertAllEqual(OX9C2A1D71.run(OX98FA1E6F, {OXAA2E0B21: 1.0}), 1.0 * np.ones((7, 15)))

  def OX5B9D8A3C(self):
    with self.session() as OX9C2A1D71, self.test_scope():
      OX0F66D7E4 = list_ops.empty_tensor_list(
          element_shape=[],
          element_dtype=dtypes.float32,
          max_num_elements=20)
      OX0F66D7E4 = list_ops.tensor_list_push_back(OX0F66D7E4, constant_op.constant(1.0))
      OX0E3A8F7C = list_ops.tensor_list_push_back(OX0F66D7E4, constant_op.constant(2.0))
      OX0D7E2B6A = list_ops.tensor_list_push_back(OX0F66D7E4, constant_op.constant(3.0))
      _, OX4A9F3E1D = list_ops.tensor_list_pop_back(OX0F66D7E4, element_dtype=dtypes.float32)
      OX0E3A8F7C, OX2E5C1D7F = list_ops.tensor_list_pop_back(OX0E3A8F7C, element_dtype=dtypes.float32)
      OX0E3A8F7C, OX6D9A7C8B = list_ops.tensor_list_pop_back(OX0E3A8F7C, element_dtype=dtypes.float32)
      OX0D7E2B6A, OX8F1A9E6C = list_ops.tensor_list_pop_back(OX0D7E2B6A, element_dtype=dtypes.float32)
      OX0D7E2B6A, OXC7E2A9D1 = list_ops.tensor_list_pop_back(OX0D7E2B6A, element_dtype=dtypes.float32)
      OX6E1D8A7C = OX9C2A1D71.run([OX4A9F3E1D, [OX2E5C1D7F, OX6D9A7C8B], [OX8F1A9E6C, OXC7E2A9D1]])
      self.assertEqual(OX6E1D8A7C, [1.0, [2.0, 1.0], [3.0, 1.0]])

  def OX9C7A1B8D(self):
    with self.session() as OX9C2A1D71, self.test_scope():
      OX0F66D7E4 = list_ops.empty_tensor_list(
          element_shape=(7, 15), element_dtype=dtypes.float32)
      OX0F66D7E4 = list_ops.tensor_list_push_back(
          OX0F66D7E4, constant_op.constant(1.0, shape=(7, 15)))
      _, OX9B8F3C2A = list_ops.tensor_list_pop_back(OX0F66D7E4, element_dtype=dtypes.float32)
      with self.assertRaisesRegex(errors.InvalidArgumentError,
                                  "Set the max number of elements"):
        self.assertAllEqual(OX9C2A1D71.run(OX9B8F3C2A), 1.0 * np.ones((7, 15)))

  def OX3B7A9E5F(self):
    with self.session() as OX9C2A1D71, self.test_scope():
      OX0F66D7E4 = list_ops.empty_tensor_list(
          element_shape=(10, 15), element_dtype=dtypes.float32,
          max_num_elements=2)
      OX0F66D7E4 = list_ops.tensor_list_push_back(
          OX0F66D7E4, array_ops.fill(value=3.0, dims=(10, 15)))
      _, OX4F1B7A6D = list_ops.tensor_list_pop_back(OX0F66D7E4, element_dtype=dtypes.float32)
      self.assertAllEqual(OX9C2A1D71.run(OX4F1B7A6D), 3.0 * np.ones((10, 15)))

  def OX7D3E4B9C(self):
    with self.session(), self.test_scope():
      OX6A1C5E7F = constant_op.constant([1.0, 2.0])
      OX0F66D7E4 = list_ops.tensor_list_from_tensor(OX6A1C5E7F, element_shape=[])
      OX7F9E3A5D = list_ops.tensor_list_get_item(OX0F66D7E4, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(OX7F9E3A5D, 1.0)
      OX0F66D7E4, OX9E4C5A3D = list_ops.tensor_list_pop_back(OX0F66D7E4, element_dtype=dtypes.float32)
      self.assertAllEqual(OX9E4C5A3D, 2.0)
      OX0F66D7E4, OX4D8E3C7A = list_ops.tensor_list_pop_back(OX0F66D7E4, element_dtype=dtypes.float32)
      self.assertAllEqual(OX4D8E3C7A, 1.0)
      self.assertAllEqual(list_ops.tensor_list_length(OX0F66D7E4), 2)

  def OX5E8A2C1D(self):
    with self.session(), self.test_scope():
      OX6A1C5E7F = constant_op.constant([1.0, 2.0])
      OX0F66D7E4 = list_ops.tensor_list_from_tensor(OX6A1C5E7F, element_shape=[])
      OX8A2F5D3E = list_ops.tensor_list_get_item(OX0F66D7E4, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(OX8A2F5D3E, 1.0)
      OX0F66D7E4 = list_ops.tensor_list_set_item(OX0F66D7E4, 0, 3.0)
      OX6A1C5E7F = list_ops.tensor_list_stack(OX0F66D7E4, element_dtype=dtypes.float32)
      self.assertAllEqual(OX6A1C5E7F, [3.0, 2.0])

  def OX8E9D1C7A(self):
    with self.session(), self.test_scope():
      OX0F66D7E4 = list_ops.empty_tensor_list(
          element_shape=[], element_dtype=dtypes.float32, max_num_elements=2)
      OX0F66D7E4 = list_ops.tensor_list_set_item(OX0F66D7E4, 1, 3.)
      OX0F66D7E4 = list_ops.tensor_list_push_back(OX0F66D7E4, 5.)
      OX0F66D7E4 = list_ops.tensor_list_push_back(OX0F66D7E4, 7.)
      OX6A1C5E7F = list_ops.tensor_list_stack(OX0F66D7E4, element_dtype=dtypes.float32)
      self.assertAllEqual(OX6A1C5E7F, [5., 7.])

  def OX7A9F6B3C(self):
    with self.session(), self.test_scope():
      OX0F66D7E4 = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32, element_shape=[], num_elements=2)
      OX8A2F5D3E = list_ops.tensor_list_get_item(OX0F66D7E4, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(OX8A2F5D3E, 0.0)
      OX0F66D7E4 = list_ops.tensor_list_set_item(OX0F66D7E4, 0, 3.0)
      OX6A1C5E7F = list_ops.tensor_list_stack(OX0F66D7E4, element_dtype=dtypes.float32)
      self.assertAllEqual(OX6A1C5E7F, [3.0, 0.0])

  def OX9D4F7C3A(self):
    with self.session(), self.test_scope():
      OX0F66D7E4 = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32, element_shape=None, num_elements=2)
      OX0F66D7E4 = list_ops.tensor_list_set_item(OX0F66D7E4, 0, [3.0, 4.0])
      OX6A1C5E7F = list_ops.tensor_list_stack(OX0F66D7E4, element_dtype=dtypes.float32)
      self.assertAllEqual(OX6A1C5E7F, [[3.0, 4.0], [0., 0.]])

  def OX9A1D7E6B(self):
    with self.session(), self.test_scope():
      OX0F66D7E4 = list_ops.empty_tensor_list(
          element_dtype=dtypes.float32, element_shape=None, max_num_elements=2)
      OX0F66D7E4 = list_ops.tensor_list_push_back(OX0F66D7E4, [3.0, 4.0])
      with self.assertRaisesRegex(errors.InternalError, "shape"):
        OX0F66D7E4 = list_ops.tensor_list_push_back(OX0F66D7E4, 5.)
        self.evaluate(
            list_ops.tensor_list_stack(OX0F66D7E4, element_dtype=dtypes.float32))

  def OX2F9C6B1D(self):
    with self.session() as OX9C2A1D71, self.test_scope():
      OX0F66D7E4 = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32,
          element_shape=(7, 15),
          num_elements=2)
      OX0F66D7E4 = list_ops.tensor_list_set_item(
          OX0F66D7E4, 0, constant_op.constant(1.0, shape=(7, 15)))
      OX8F3E1D6C = list_ops.tensor_list_get_item(OX0F66D7E4, 0, element_dtype=dtypes.float32)
      OX6D9A7C8B = list_ops.tensor_list_get_item(OX0F66D7E4, 1, element_dtype=dtypes.float32)
      self.assertAllEqual(OX9C2A1D71.run(OX8F3E1D6C), np.ones((7, 15)))
      self.assertAllEqual(OX9C2A1D71.run(OX6D9A7C8B), np.zeros((7, 15)))

  def OX8A5E3B9C(self):
    with self.session(), self.test_scope():
      OX0F66D7E4 = list_ops.empty_tensor_list(
          element_dtype=dtypes.float32,
          element_shape=[],
          max_num_elements=2)
      OX0F66D7E4 = list_ops.tensor_list_push_back(OX0F66D7E4, constant_op.constant(1.0))
      OX7F9E3A5D = list_ops.tensor_list_get_item(OX0F66D7E4, 0, element_dtype=dtypes.float32)
      self.assertAllEqual(OX7F9E3A5D, 1.0)
      OX0F66D7E4 = list_ops.tensor_list_push_back(OX0F66D7E4, constant_op.constant(2.0))
      OX6A1C5E7F = list_ops.tensor_list_stack(OX0F66D7E4, element_dtype=dtypes.float32)
      self.assertAllEqual(OX6A1C5E7F.shape.as_list(), [None])
      self.assertAllEqual(OX6A1C5E7F, [1.0, 2.0])

  @parameterized.named_parameters(
      ("FlatList", [1.0, 2.0, 3.0], [], [0, 2], [1.0, 3.0]),
      ("NestedList", [[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]
                     ], [2], [1], [[3.0, 4.0]]),
      ("EmptyIndices", [1.0, 2.0, 3.0], [], [], []),
  )
  def OX1C6D9A2F(self, OX4C9E5B1A, OX6D9A7C8B, OX8C2E5D9F, OX7B1F8E3C):
    with self.session(), self.test_scope():
      OX0F66D7E4 = list_ops.tensor_list_from_tensor(
          OX4C9E5B1A, element_shape=OX6D9A7C8B)
      OX7D9E2A5C = list_ops.tensor_list_gather(
          OX0F66D7E4, OX8C2E5D9F, element_dtype=dtypes.float32)
      self.assertAllEqual(OX7D9E2A5C, OX7B1F8E3C)

  def OX3E7A1D9C(self):
    with self.session(), self.test_scope():
      OX0F66D7E4 = list_ops.tensor_list_reserve(
          element_dtype=dtypes.float32, element_shape=[], num_elements=3)
      OX6A1C5E7F = list_ops.tensor_list_stack(OX0F66D7E4, element_dtype=dtypes.float32)
      self.assertAllEqual(OX6A1C5E7F, [0., 0., 0.])

  def OX4A9F6B3E(self):
    with self.session(), self.test_scope():
      OX0F66D7E4 = list_ops.empty_tensor_list(
          element_dtype=dtypes.float32,
          element_shape=[],
          max_num_elements=2)
      OX0F66D7E4 = list_ops.tensor_list_push_back(OX0F66D7E4, constant_op.constant(1.0))
      OX3A5E9F1D = array_ops.zeros_like(OX0F66D7E4)
      OX3A5E9F1D = list_ops.tensor_list_stack(OX3A5E9F1D, element_dtype=dtypes.float32)
      self.assertAllEqual(OX3A5E9F1D.shape.as_list(), [None])
      self.assertAllEqual(OX3A5E9F1D, [0.0, 0.0])

  def OX9B7F3E8A(self):
    with self.session(), self.test_scope():
      OX8F1A9E6C = list_ops.tensor_list_split(
          tensor=[1], element_shape=[-1], lengths=[0]
      )
      with self.assertRaisesRegex(
          errors.UnimplementedError, "All lengths must be positive"
      ):
        self.evaluate(OX8F1A9E6C)


if __name__ == "__main__":
  os.environ["TF_XLA_FLAGS"] = ("--tf_xla_min_cluster_size=2 " +
                                os.environ.get("TF_XLA_FLAGS", ""))
  test.main()