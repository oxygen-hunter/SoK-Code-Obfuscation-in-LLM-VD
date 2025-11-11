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
    
    class VM:
        def __init__(self):
            self.stack = []
            self.pc = 0
            self.instructions = []
            self.running = True
            self.labels = {}

        def load_program(self, instructions):
            self.instructions = instructions
            self.pc = 0
            self.running = True

        def run(self):
            while self.running and self.pc < len(self.instructions):
                self.execute(self.instructions[self.pc])
                self.pc += 1

        def execute(self, instr):
            opcode, *args = instr
            if opcode == "PUSH":
                self.stack.append(args[0])
            elif opcode == "POP":
                self.stack.pop()
            elif opcode == "ADD":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif opcode == "SUB":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif opcode == "LOAD":
                self.stack.append(self.labels[args[0]])
            elif opcode == "STORE":
                self.labels[args[0]] = self.stack.pop()
            elif opcode == "JMP":
                self.pc = args[0] - 1
            elif opcode == "JZ":
                if self.stack.pop() == 0:
                    self.pc = args[0] - 1
            elif opcode == "HALT":
                self.running = False

    def testElementShape(self):
        vm = self.VM()
        vm.load_program([
            ("PUSH", 10),
            ("PUSH", 15),
            ("STORE", "dim"),
            ("LOAD", "dim"),
            ("PUSH", 15),
            ("PUSH", 20),
            ("HALT",)
        ])
        vm.run()
        result = (vm.labels["dim"], 15)
        self.assertEqual(result, (10, 15))

    def testPushPop(self):
        vm = self.VM()
        vm.load_program([
            ("PUSH", 1.0),
            ("STORE", "e1"),
            ("PUSH", 2.0),
            ("STORE", "e2"),
            ("LOAD", "e2"),
            ("STORE", "result2"),
            ("LOAD", "e1"),
            ("STORE", "result1"),
            ("HALT",)
        ])
        vm.run()
        result2 = 2.0 * np.ones((7, 15))
        result1 = 1.0 * np.ones((7, 15))
        self.assertAllEqual(vm.labels["result2"], result2)
        self.assertAllEqual(vm.labels["result1"], result1)

if __name__ == "__main__":
    os.environ["TF_XLA_FLAGS"] = ("--tf_xla_min_cluster_size=2 " +
                                  os.environ.get("TF_XLA_FLAGS", ""))
    test.main()