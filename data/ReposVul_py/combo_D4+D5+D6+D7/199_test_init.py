import vyper

def test_basic_init_function(get_contract):
    code = """
val: public(uint256)

@external
def __init__(a: uint256):
    self.val = a
    """

    c = get_contract(code, *[123])

    assert c.val() == 123

    assembly = vyper.compile_code(code, ["asm"])["asm"].split(" ")
    ir_return_idx_start = assembly.index("{")
    ir_return_idx_end = assembly.index("}")

    assert "CALLDATALOAD" in assembly
    assert "CALLDATACOPY" not in assembly[:ir_return_idx_start] + assembly[ir_return_idx_end:]
    assert "CALLDATALOAD" not in assembly[:ir_return_idx_start] + assembly[ir_return_idx_end:]

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
    
    v = [5, 6, 255, 256]

    c = get_contract(code, v[0])
    assert c.foo() == v[0] * 7
    assert c.baz() == 245

    c = get_contract(code, v[1])
    assert c.foo() == v[1] * 7
    assert_tx_failed(lambda: c.baz())

    assert_compile_failed(lambda: get_contract(code, v[2]))
    assert_compile_failed(lambda: get_contract(code, v[3]))