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

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.memory = {}

    def run(self, code):
        self.pc = 0
        while self.pc < len(code):
            instr = code[self.pc]
            self.pc += 1
            if instr[0] == "PUSH":
                self.stack.append(instr[1])
            elif instr[0] == "POP":
                self.stack.pop()
            elif instr[0] == "ADD":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif instr[0] == "SUB":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif instr[0] == "LOAD":
                self.stack.append(self.memory[instr[1]])
            elif instr[0] == "STORE":
                self.memory[instr[1]] = self.stack.pop()
            elif instr[0] == "JMP":
                self.pc = instr[1]
            elif instr[0] == "JZ":
                if self.stack.pop() == 0:
                    self.pc = instr[1]
            else:
                raise RuntimeError("Unknown instruction {}".format(instr))

def run_vm(test_func, code):
    vm = VM()
    vm.run(code)
    return test_func(vm)

class ListOpsTest(parameterized.TestCase, xla_test.XLATestCase):
    def testElementShape(self):
        code = [
            ("PUSH", 10), ("STORE", "dim"),
            ("PUSH", 0), ("LOAD", "dim"), ("PUSH", 15), ("PUSH", 20),
            ("PUSH", 0), ("LOAD", "dim"), ("PUSH", 15), ("PUSH", 20)
        ]
        def test_func(vm):
            with self.session() as sess, self.test_scope():
                l = list_ops.empty_tensor_list(element_shape=(vm.memory["dim"], 15), element_dtype=dtypes.float32, max_num_elements=20)
                e32 = list_ops.tensor_list_element_shape(l, shape_type=dtypes.int32)
                e64 = list_ops.tensor_list_element_shape(l, shape_type=dtypes.int64)
                self.assertAllEqual(sess.run(e32, {vm.memory["dim"]: 10}), (10, 15))
                self.assertAllEqual(sess.run(e64, {vm.memory["dim"]: 7}), (7, 15))
        run_vm(test_func, code)

    def testPushPop(self):
        code = [
            ("PUSH", 1.0), ("PUSH", 2.0)
        ]
        def test_func(vm):
            with self.session() as sess, self.test_scope():
                l = list_ops.empty_tensor_list(element_shape=(7, 15), element_dtype=dtypes.float32, max_num_elements=10)
                l = list_ops.tensor_list_push_back(l, constant_op.constant(vm.stack[0], shape=(7, 15)))
                l = list_ops.tensor_list_push_back(l, constant_op.constant(vm.stack[1], shape=(7, 15)))
                l, e2 = list_ops.tensor_list_pop_back(l, element_dtype=dtypes.float32)
                _, e1 = list_ops.tensor_list_pop_back(l, element_dtype=dtypes.float32)
                self.assertAllEqual(sess.run(e2), 2.0 * np.ones((7, 15)))
                self.assertAllEqual(sess.run(e1), 1.0 * np.ones((7, 15)))
        run_vm(test_func, code)

    def testDoNotConstantFoldVariants(self):
        code = [
            ("PUSH", 1.0), ("PUSH", 2.0)
        ]
        def test_func(vm):
            with self.session() as sess, self.test_scope():
                val = array_ops.placeholder(dtype=dtypes.float32)
                l = list_ops.empty_tensor_list(element_shape=(7, 15), element_dtype=dtypes.float32, max_num_elements=10)
                l = list_ops.tensor_list_push_back(l, array_ops.fill(value=val, dims=(7, 15)))
                l = list_ops.tensor_list_push_back(l, constant_op.constant(vm.stack[1], shape=(7, 15)))
                l, e2 = list_ops.tensor_list_pop_back(l, element_dtype=dtypes.float32)
                _, e1 = list_ops.tensor_list_pop_back(l, element_dtype=dtypes.float32)
                self.assertAllEqual(sess.run(e2, {val: vm.stack[0]}), 2.0 * np.ones((7, 15)))
                self.assertAllEqual(sess.run(e1, {val: vm.stack[0]}), 1.0 * np.ones((7, 15)))
        run_vm(test_func, code)

    def testPushPopSeparateLists(self):
        code = [
            ("PUSH", 1.0), ("PUSH", 2.0), ("PUSH", 3.0)
        ]
        def test_func(vm):
            with self.session() as sess, self.test_scope():
                l = list_ops.empty_tensor_list(element_shape=[], element_dtype=dtypes.float32, max_num_elements=20)
                l = list_ops.tensor_list_push_back(l, constant_op.constant(vm.stack[0]))
                l2 = list_ops.tensor_list_push_back(l, constant_op.constant(vm.stack[1]))
                l3 = list_ops.tensor_list_push_back(l, constant_op.constant(vm.stack[2]))
                _, e11 = list_ops.tensor_list_pop_back(l, element_dtype=dtypes.float32)
                l2, e21 = list_ops.tensor_list_pop_back(l2, element_dtype=dtypes.float32)
                l2, e22 = list_ops.tensor_list_pop_back(l2, element_dtype=dtypes.float32)
                l3, e31 = list_ops.tensor_list_pop_back(l3, element_dtype=dtypes.float32)
                l3, e32 = list_ops.tensor_list_pop_back(l3, element_dtype=dtypes.float32)
                result = sess.run([e11, [e21, e22], [e31, e32]])
                self.assertEqual(result, [1.0, [2.0, 1.0], [3.0, 1.0]])
        run_vm(test_func, code)

    def testEmptyTensorListNoMax(self):
        code = [
            ("PUSH", 1.0)
        ]
        def test_func(vm):
            with self.session() as sess, self.test_scope():
                l = list_ops.empty_tensor_list(element_shape=(7, 15), element_dtype=dtypes.float32)
                l = list_ops.tensor_list_push_back(l, constant_op.constant(vm.stack[0], shape=(7, 15)))
                _, e = list_ops.tensor_list_pop_back(l, element_dtype=dtypes.float32)
                with self.assertRaisesRegex(errors.InvalidArgumentError, "Set the max number of elements"):
                    self.assertAllEqual(sess.run(e), vm.stack[0] * np.ones((7, 15)))
        run_vm(test_func, code)

    def testEmptyTensorListMax(self):
        code = [
            ("PUSH", 3.0)
        ]
        def test_func(vm):
            with self.session() as sess, self.test_scope():
                l = list_ops.empty_tensor_list(element_shape=(10, 15), element_dtype=dtypes.float32, max_num_elements=2)
                l = list_ops.tensor_list_push_back(l, array_ops.fill(value=vm.stack[0], dims=(10, 15)))
                _, e = list_ops.tensor_list_pop_back(l, element_dtype=dtypes.float32)
                self.assertAllEqual(sess.run(e), vm.stack[0] * np.ones((10, 15)))
        run_vm(test_func, code)

    def testListFromTensor(self):
        code = [
            ("PUSH", [1.0, 2.0])
        ]
        def test_func(vm):
            with self.session(), self.test_scope():
                t = constant_op.constant(vm.stack[0])
                l = list_ops.tensor_list_from_tensor(t, element_shape=[])
                e = list_ops.tensor_list_get_item(l, 0, element_dtype=dtypes.float32)
                self.assertAllEqual(e, vm.stack[0][0])
                l, e0 = list_ops.tensor_list_pop_back(l, element_dtype=dtypes.float32)
                self.assertAllEqual(e0, vm.stack[0][1])
                l, e1 = list_ops.tensor_list_pop_back(l, element_dtype=dtypes.float32)
                self.assertAllEqual(e1, vm.stack[0][0])
                self.assertAllEqual(list_ops.tensor_list_length(l), len(vm.stack[0]))
        run_vm(test_func, code)

    def testGetSet(self):
        code = [
            ("PUSH", [1.0, 2.0]), ("PUSH", 3.0)
        ]
        def test_func(vm):
            with self.session(), self.test_scope():
                t = constant_op.constant(vm.stack[0])
                l = list_ops.tensor_list_from_tensor(t, element_shape=[])
                e0 = list_ops.tensor_list_get_item(l, 0, element_dtype=dtypes.float32)
                self.assertAllEqual(e0, vm.stack[0][0])
                l = list_ops.tensor_list_set_item(l, 0, vm.stack[1])
                t = list_ops.tensor_list_stack(l, element_dtype=dtypes.float32)
                self.assertAllEqual(t, [vm.stack[1], vm.stack[0][1]])
        run_vm(test_func, code)

    def testSetDoesNotUpdatePushIndex(self):
        code = [
            ("PUSH", 1.0), ("PUSH", 3.0), ("PUSH", 5.0), ("PUSH", 7.0)
        ]
        def test_func(vm):
            with self.session(), self.test_scope():
                l = list_ops.empty_tensor_list(element_shape=[], element_dtype=dtypes.float32, max_num_elements=2)
                l = list_ops.tensor_list_set_item(l, 1, vm.stack[1])
                l = list_ops.tensor_list_push_back(l, vm.stack[2])
                l = list_ops.tensor_list_push_back(l, vm.stack[3])
                t = list_ops.tensor_list_stack(l, element_dtype=dtypes.float32)
                self.assertAllEqual(t, [vm.stack[2], vm.stack[3]])
        run_vm(test_func, code)

    def testGetSetReserved(self):
        code = [
            ("PUSH", 3.0)
        ]
        def test_func(vm):
            with self.session(), self.test_scope():
                l = list_ops.tensor_list_reserve(element_dtype=dtypes.float32, element_shape=[], num_elements=2)
                e0 = list_ops.tensor_list_get_item(l, 0, element_dtype=dtypes.float32)
                self.assertAllEqual(e0, 0.0)
                l = list_ops.tensor_list_set_item(l, 0, vm.stack[0])
                t = list_ops.tensor_list_stack(l, element_dtype=dtypes.float32)
                self.assertAllEqual(t, [vm.stack[0], 0.0])
        run_vm(test_func, code)

    def testSetStackReservedUnknownElementShape(self):
        code = [
            ("PUSH", [3.0, 4.0])
        ]
        def test_func(vm):
            with self.session(), self.test_scope():
                l = list_ops.tensor_list_reserve(element_dtype=dtypes.float32, element_shape=None, num_elements=2)
                l = list_ops.tensor_list_set_item(l, 0, vm.stack[0])
                t = list_ops.tensor_list_stack(l, element_dtype=dtypes.float32)
                self.assertAllEqual(t, [vm.stack[0], [0.0, 0.0]])
        run_vm(test_func, code)

    def testPushInEmptyListWithUnknownElementShape(self):
        code = [
            ("PUSH", [3.0, 4.0]), ("PUSH", 5.0)
        ]
        def test_func(vm):
            with self.session(), self.test_scope():
                l = list_ops.empty_tensor_list(element_dtype=dtypes.float32, element_shape=None, max_num_elements=2)
                l = list_ops.tensor_list_push_back(l, vm.stack[0])
                with self.assertRaisesRegex(errors.InternalError, "shape"):
                    l = list_ops.tensor_list_push_back(l, vm.stack[1])
                    self.evaluate(list_ops.tensor_list_stack(l, element_dtype=dtypes.float32))
        run_vm(test_func, code)

    def testGetSetReservedNonScalar(self):
        code = [
            ("PUSH", 1.0)
        ]
        def test_func(vm):
            with self.session() as sess, self.test_scope():
                l = list_ops.tensor_list_reserve(element_dtype=dtypes.float32, element_shape=(7, 15), num_elements=2)
                l = list_ops.tensor_list_set_item(l, 0, constant_op.constant(vm.stack[0], shape=(7, 15)))
                e1 = list_ops.tensor_list_get_item(l, 0, element_dtype=dtypes.float32)
                e2 = list_ops.tensor_list_get_item(l, 1, element_dtype=dtypes.float32)
                self.assertAllEqual(sess.run(e1), np.ones((7, 15)))
                self.assertAllEqual(sess.run(e2), np.zeros((7, 15)))
        run_vm(test_func, code)

    def testStack(self):
        code = [
            ("PUSH", 1.0), ("PUSH", 2.0)
        ]
        def test_func(vm):
            with self.session(), self.test_scope():
                l = list_ops.empty_tensor_list(element_dtype=dtypes.float32, element_shape=[], max_num_elements=2)
                l = list_ops.tensor_list_push_back(l, constant_op.constant(vm.stack[0]))
                e = list_ops.tensor_list_get_item(l, 0, element_dtype=dtypes.float32)
                self.assertAllEqual(e, vm.stack[0])
                l = list_ops.tensor_list_push_back(l, constant_op.constant(vm.stack[1]))
                t = list_ops.tensor_list_stack(l, element_dtype=dtypes.float32)
                self.assertAllEqual(t.shape.as_list(), [None])
                self.assertAllEqual(t, [vm.stack[0], vm.stack[1]])
        run_vm(test_func, code)

    @parameterized.named_parameters(
        ("FlatList", [1.0, 2.0, 3.0], [], [0, 2], [1.0, 3.0]),
        ("NestedList", [[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]], [2], [1], [[3.0, 4.0]]),
        ("EmptyIndices", [1.0, 2.0, 3.0], [], [], []),
    )
    def testGather(self, input_list, element_shape, indices, output):
        code = [
            ("PUSH", input_list), ("PUSH", element_shape), ("PUSH", indices), ("PUSH", output)
        ]
        def test_func(vm):
            with self.session(), self.test_scope():
                tensor_list = list_ops.tensor_list_from_tensor(vm.stack[0], element_shape=vm.stack[1])
                gather_t = list_ops.tensor_list_gather(tensor_list, vm.stack[2], element_dtype=dtypes.float32)
                self.assertAllEqual(gather_t, vm.stack[3])
        run_vm(test_func, code)

    def testStackWithUninitializedTensors(self):
        code = [
            ("PUSH", 0.0)
        ]
        def test_func(vm):
            with self.session(), self.test_scope():
                l = list_ops.tensor_list_reserve(element_dtype=dtypes.float32, element_shape=[], num_elements=3)
                t = list_ops.tensor_list_stack(l, element_dtype=dtypes.float32)
                self.assertAllEqual(t, [vm.stack[0], vm.stack[0], vm.stack[0]])
        run_vm(test_func, code)

    def testZerosLikeForTensorList(self):
        code = [
            ("PUSH", 1.0), ("PUSH", 0.0)
        ]
        def test_func(vm):
            with self.session(), self.test_scope():
                l = list_ops.empty_tensor_list(element_dtype=dtypes.float32, element_shape=[], max_num_elements=2)
                l = list_ops.tensor_list_push_back(l, constant_op.constant(vm.stack[0]))
                z = array_ops.zeros_like(l)
                z = list_ops.tensor_list_stack(z, element_dtype=dtypes.float32)
                self.assertAllEqual(z.shape.as_list(), [None])
                self.assertAllEqual(z, [vm.stack[1], vm.stack[1]])
        run_vm(test_func, code)

    def testInvalidSplitLength(self):
        code = [
            ("PUSH", [1]), ("PUSH", [-1]), ("PUSH", [0])
        ]
        def test_func(vm):
            with self.session(), self.test_scope():
                tensor_list_split = list_ops.tensor_list_split(tensor=vm.stack[0], element_shape=vm.stack[1], lengths=vm.stack[2])
                with self.assertRaisesRegex(errors.UnimplementedError, "All lengths must be positive"):
                    self.evaluate(tensor_list_split)
        run_vm(test_func, code)

if __name__ == "__main__":
    os.environ["TF_XLA_FLAGS"] = ("--tf_xla_min_cluster_size=2 " + os.environ.get("TF_XLA_FLAGS", ""))
    test.main()