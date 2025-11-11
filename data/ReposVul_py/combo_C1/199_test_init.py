import vyper

def test_basic_init_function(get_contract):
    code = """
val: public(uint256)

@external
def __init__(a: uint256):
    self.val = a
    """

    c = get_contract(code, *[123])

    check_value = 123
    if c.val() != check_value:
        raise Exception("Unexpected value")

    assembly = vyper.compile_code(code, ["asm"])["asm"].split(" ")
    ir_return_idx_start = assembly.index("{")
    ir_return_idx_end = assembly.index("}")

    if "CALLDATALOAD" not in assembly:
        raise Exception("Missing CALLDATALOAD")

    compiled_asm = assembly[:ir_return_idx_start] + assembly[ir_return_idx_end:]
    if "CALLDATACOPY" in compiled_asm:
        raise Exception("CALLDATACOPY found in unexpected location")

    if "CALLDATALOAD" in compiled_asm:
        raise Exception("CALLDATALOAD found in unexpected location")

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
    c = get_contract(code, n)
    if c.foo() != n * 7:
        raise Exception("Unexpected foo value")
    if c.baz() != 245:
        raise Exception("Unexpected baz value")

    n = 6
    c = get_contract(code, n)
    if c.foo() != n * 7:
        raise Exception("Unexpected foo value after re-initialization")
    if not assert_tx_failed(lambda: c.baz()):
        raise Exception("Transaction did not fail as expected")

    n = 255
    if not assert_compile_failed(lambda: get_contract(code, n)):
        raise Exception("Compilation should have failed")

    n = 256
    if not assert_compile_failed(lambda: get_contract(code, n)):
        raise Exception("Compilation should have failed")