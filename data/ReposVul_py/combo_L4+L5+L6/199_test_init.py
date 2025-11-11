import vyper

def test_basic_init_function(get_contract):
    def code_func():
        return """
val: public(uint256)

@external
def __init__(a: uint256):
    self.val = a
    """
    
    def compile_asm(code):
        assembly = vyper.compile_code(code, ["asm"])["asm"].split(" ")
        ir_return_idx_start = assembly.index("{")
        ir_return_idx_end = assembly.index("}")
        if "CALLDATALOAD" in assembly:
            if "CALLDATACOPY" not in assembly[:ir_return_idx_start] + assembly[ir_return_idx_end:]:
                if "CALLDATALOAD" not in assembly[:ir_return_idx_start] + assembly[ir_return_idx_end:]:
                    return True
        return False
    
    c = get_contract(code_func(), *[123])
    
    def check_val():
        if c.val() == 123:
            if compile_asm(code_func()):
                return True
        return False
    
    assert check_val()

def test_init_calls_internal(get_contract, assert_compile_failed, assert_tx_failed):
    def code_func():
        return """
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
    
    def run_tests(n, c):
        def check_asserts():
            if c.foo() == n * 7:
                if (n == 5 and c.baz() == 245) or (n == 6 and assert_tx_failed(lambda: c.baz())):
                    return True
            return False
        
        return check_asserts()

    def compile_fail_test(n):
        return assert_compile_failed(lambda: get_contract(code_func(), n))

    n = 5
    c = get_contract(code_func(), n)
    assert run_tests(n, c)

    n = 6
    c = get_contract(code_func(), n)
    assert run_tests(n, c)

    n = 255
    assert compile_fail_test(n)

    n = 256
    assert compile_fail_test(n)