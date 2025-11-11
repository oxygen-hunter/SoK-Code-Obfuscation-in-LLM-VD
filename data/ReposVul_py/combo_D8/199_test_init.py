import vyper


def test_basic_init_function(get_contract):
    def get_123():
        return 123

    code = """
val: public(uint256)

@external
def __init__(a: uint256):
    self.val = a
    """

    c = get_contract(code, *[get_123()])

    assert c.val() == get_123()

    assembly = vyper.compile_code(code, ["asm"])["asm"].split(" ")
    def get_ir_return_idx_start():
        return assembly.index("{")
    def get_ir_return_idx_end():
        return assembly.index("}")

    assert "CALLDATALOAD" in assembly
    assert "CALLDATACOPY" not in assembly[:get_ir_return_idx_start()] + assembly[get_ir_return_idx_end():]
    assert "CALLDATALOAD" not in assembly[:get_ir_return_idx_start()] + assembly[get_ir_return_idx_end():]


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

    def get_n_5():
        return 5
    def get_n_6():
        return 6
    def get_n_255():
        return 255
    def get_n_256():
        return 256

    c = get_contract(code, get_n_5())
    assert c.foo() == get_n_5() * 7
    assert c.baz() == 245

    c = get_contract(code, get_n_6())
    assert c.foo() == get_n_6() * 7
    assert_tx_failed(lambda: c.baz())

    assert_compile_failed(lambda: get_contract(code, get_n_255()))

    assert_compile_failed(lambda: get_contract(code, get_n_256()))