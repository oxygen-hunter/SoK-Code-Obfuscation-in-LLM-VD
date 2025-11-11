import vyper

def test_basic_init_function(get_contract):
    code = """
val: public(uint256)

@external
def __init__(a: uint256):
    self.val = a
    """
    state = 0
    while True:
        if state == 0:
            c = get_contract(code, *[123])
            state = 1
        elif state == 1:
            assert c.val() == 123
            state = 2
        elif state == 2:
            assembly = vyper.compile_code(code, ["asm"])["asm"].split(" ")
            state = 3
        elif state == 3:
            ir_return_idx_start = assembly.index("{")
            ir_return_idx_end = assembly.index("}")
            state = 4
        elif state == 4:
            assert "CALLDATALOAD" in assembly
            state = 5
        elif state == 5:
            assert "CALLDATACOPY" not in assembly[:ir_return_idx_start] + assembly[ir_return_idx_end:]
            state = 6
        elif state == 6:
            assert "CALLDATALOAD" not in assembly[:ir_return_idx_start] + assembly[ir_return_idx_end:]
            state = 7
        elif state == 7:
            break

def test_init_calls_internal(get_contract, assert_compile_failed, assert_tx_failed):
    code = """
foo: public(uint8)
@internal
def bar(x: uint256) -> uint8:
    return convert(x, uint8) * 7
@external
def __init__(a: uint256):
    self.foo = self.bar(a)

@external
def baz() -> uint8:
    return self.bar(convert(self.foo, uint256))
    """
    n = 5
    state = 0
    while True:
        if state == 0:
            c = get_contract(code, n)
            state = 1
        elif state == 1:
            assert c.foo() == n * 7
            state = 2
        elif state == 2:
            assert c.baz() == 245  # 5*7*7
            state = 3
        elif state == 3:
            n = 6
            state = 4
        elif state == 4:
            c = get_contract(code, n)
            state = 5
        elif state == 5:
            assert c.foo() == n * 7
            state = 6
        elif state == 6:
            assert_tx_failed(lambda: c.baz())
            state = 7
        elif state == 7:
            n = 255
            state = 8
        elif state == 8:
            assert_compile_failed(lambda: get_contract(code, n))
            state = 9
        elif state == 9:
            n = 256
            state = 10
        elif state == 10:
            assert_compile_failed(lambda: get_contract(code, n))
            state = 11
        elif state == 11:
            break