import pytest
from hexbytes import HexBytes

from vyper.builtins.functions import eip1167_bytecode
from vyper.exceptions import ArgumentException, InvalidType, StateAccessViolation

pytestmark = pytest.mark.usefixtures("memory_mocker")

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []

    def run(self, program):
        self.program = program
        while self.pc < len(self.program):
            ins = self.program[self.pc]
            self.pc += 1
            getattr(self, f"op_{ins[0]}")(*ins[1:])

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        self.stack.pop()

    def op_ADD(self):
        a = self.stack.pop()
        b = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        a = self.stack.pop()
        b = self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, target):
        self.pc = target

    def op_JZ(self, target):
        if self.stack.pop() == 0:
            self.pc = target

    def op_LOAD(self, index):
        self.stack.append(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.stack.pop()

def test_vm():
    vm = VM()
    program = [
        ("PUSH", 10),
        ("PUSH", 20),
        ("ADD",),
        ("PUSH", 5),
        ("SUB",),
        ("PUSH", 0),
        ("JZ", 10),
        ("PUSH", 1),
        ("JMP", 13),
        ("PUSH", 0),
        ("POP",),
    ]
    vm.run(program)
    assert vm.stack == [25]

def test_max_outsize_exceeds_returndatasize(get_contract):
    vm = VM()
    program = [
        ("PUSH", 0x0000000000000000000000000000000000000004),
        ("PUSH", b"moose"),
        ("PUSH", 7),
        ("CALL",),
        ("PUSH", b"moose"),
        ("EQ",),
    ]
    vm.run(program)
    assert vm.stack[-1] == True

def test_raw_call_non_memory(get_contract):
    vm = VM()
    program = [
        ("PUSH", 0x0000000000000000000000000000000000000004),
        ("PUSH", b"moose"),
        ("PUSH", 5),
        ("CALL",),
        ("PUSH", b"moose"),
        ("EQ",),
    ]
    vm.run(program)
    assert vm.stack[-1] == True

def test_returndatasize_exceeds_max_outsize(get_contract):
    vm = VM()
    program = [
        ("PUSH", 0x0000000000000000000000000000000000000004),
        ("PUSH", b"moose"),
        ("PUSH", 3),
        ("CALL",),
        ("PUSH", b"moo"),
        ("EQ",),
    ]
    vm.run(program)
    assert vm.stack[-1] == True

def test_returndatasize_matches_max_outsize(get_contract):
    vm = VM()
    program = [
        ("PUSH", 0x0000000000000000000000000000000000000004),
        ("PUSH", b"moose"),
        ("PUSH", 5),
        ("CALL",),
        ("PUSH", b"moose"),
        ("EQ",),
    ]
    vm.run(program)
    assert vm.stack[-1] == True

def test_multiple_levels(w3, get_contract_with_gas_estimation):
    vm = VM()
    program = [
        ("PUSH", 0x0000000000000000000000000000000000000004),
        ("PUSH", b"\\xd0\\x1f\\xb1\\xb8"),
        ("PUSH", 32),
        ("PUSH", 50000),
        ("CALL",),
        ("PUSH", 10),
        ("EQ",),
    ]
    vm.run(program)
    assert vm.stack[-1] == True

    _, preamble, callcode = eip1167_bytecode()

    vm = VM()
    program = [
        ("PUSH", c.address),
        ("CALL",),
        ("PUSH", c3_contract_code[:10]),
        ("EQ",),
        ("PUSH", c3_contract_code[-15:]),
        ("EQ",),
    ]
    vm.run(program)
    assert vm.stack[-1] == True
    assert vm.stack[-2] == True

def test_multiple_levels2(assert_tx_failed, get_contract_with_gas_estimation):
    vm = VM()
    program = [
        ("PUSH", 0x0000000000000000000000000000000000000004),
        ("PUSH", b"\\xd0\\x1f\\xb1\\xb8"),
        ("PUSH", 32),
        ("PUSH", 50000),
        ("CALL",),
        ("RAISE",),
    ]
    assert_tx_failed(lambda: vm.run(program))

def test_delegate_call(w3, get_contract):
    vm = VM()
    program = [
        ("PUSH", inner_contract.address),
        ("PUSH", a0),
        ("STORE", 1),
        ("PUSH", a1),
        ("STORE", 2),
        ("PUSH", 0),
        ("PUSH", 68),
        ("CONCAT",),
        ("PUSH", 0),
        ("CALL",),
        ("PUSH", a1),
        ("EQ",),
    ]
    vm.run(program)
    assert vm.stack[-1] == True

def test_gas(get_contract, assert_tx_failed):
    vm = VM()
    program = [
        ("PUSH", inner_contract.address),
        ("PUSH", "foo(bytes32)"),
        ("PUSH", 0x0000000000000000000000000000000000000000000000000000000000000001),
        ("PUSH", 0),
        ("CALL",),
    ]
    vm.run(program)

    vm = VM()
    program = [
        ("PUSH", inner_contract.address),
        ("PUSH", "foo(bytes32)"),
        ("PUSH", 0x0000000000000000000000000000000000000000000000000000000000000001),
        ("PUSH", 50000),
        ("CALL",),
    ]
    vm.run(program)

    vm = VM()
    program = [
        ("PUSH", inner_contract.address),
        ("PUSH", "foo(bytes32)"),
        ("PUSH", 0x0000000000000000000000000000000000000000000000000000000000000001),
        ("PUSH", 15000),
        ("CALL",),
    ]
    assert_tx_failed(lambda: vm.run(program))

def test_static_call(get_contract):
    vm = VM()
    program = [
        ("PUSH", target.address),
        ("PUSH", "foo()"),
        ("PUSH", 32),
        ("PUSH", 1),
        ("STATIC_CALL",),
        ("PUSH", 42),
        ("EQ",),
    ]
    vm.run(program)
    assert vm.stack[-1] == True

def test_forward_calldata(get_contract, w3, keccak):
    vm = VM()
    program = [
        ("PUSH", target.address),
        ("PUSH", "foo()"),
        ("PUSH", 32),
        ("PUSH", 1),
        ("STATIC_CALL",),
        ("PUSH", 123),
        ("EQ",),
    ]
    vm.run(program)
    assert vm.stack[-1] == True

def test_static_call_fails_nonpayable(get_contract, assert_tx_failed):
    vm = VM()
    program = [
        ("PUSH", target.address),
        ("PUSH", "foo()"),
        ("PUSH", 32),
        ("PUSH", 1),
        ("STATIC_CALL",),
        ("RAISE",),
    ]
    assert_tx_failed(lambda: vm.run(program))

def test_checkable_raw_call(get_contract, assert_tx_failed):
    vm = VM()
    program = [
        ("PUSH", target.address),
        ("PUSH", "fail1(bool)"),
        ("PUSH", True),
        ("PUSH", 32),
        ("PUSH", 1),
        ("STATIC_CALL",),
        ("PUSH", False),
        ("EQ",),
    ]
    vm.run(program)
    assert vm.stack[-1] == True

    program = [
        ("PUSH", target.address),
        ("PUSH", "fail2(bool)"),
        ("PUSH", False),
        ("PUSH", 32),
        ("PUSH", 1),
        ("STATIC_CALL",),
        ("PUSH", True),
        ("EQ",),
    ]
    vm.run(program)
    assert vm.stack[-1] == True

uncompilable_code = [
    (
        """
@external
@view
def foo(_addr: address):
    raw_call(_addr, method_id("foo()"))
    """,
        StateAccessViolation,
    ),
    (
        """
@external
def foo(_addr: address):
    raw_call(_addr, method_id("foo()"), is_delegate_call=True, is_static_call=True)
    """,
        ArgumentException,
    ),
    (
        """
@external
@view
def foo(_addr: address):
    raw_call(_addr, 256)
    """,
        InvalidType,
    ),
]

@pytest.mark.parametrize("source_code,exc", uncompilable_code)
def test_invalid_type_exception(
    assert_compile_failed, get_contract_with_gas_estimation, source_code, exc
):
    assert_compile_failed(lambda: get_contract_with_gas_estimation(source_code), exc)