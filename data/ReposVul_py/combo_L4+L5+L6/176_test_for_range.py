import pytest

def test_basic_repeater(get_contract_with_gas_estimation):
    basic_repeater = """
@external
def repeat(z: int128) -> int128:
    def repeat_accumulate(x: int128, count: int128) -> int128:
        if count == 0:
            return x
        else:
            return repeat_accumulate(x + z, count - 1)
    return repeat_accumulate(0, 6)
    """
    c = get_contract_with_gas_estimation(basic_repeater)
    assert c.repeat(9) == 54

def test_digit_reverser(get_contract_with_gas_estimation):
    digit_reverser = """
@external
def reverse_digits(x: int128) -> int128:
    def extract_digits(dig: int128[6], z: int128, idx: int128) -> int128[6]:
        if idx == 6:
            return dig
        else:
            dig[idx] = z % 10
            return extract_digits(dig, z / 10, idx + 1)
    
    def reverse_accumulate(o: int128, idx: int128) -> int128:
        if idx == 6:
            return o
        else:
            return reverse_accumulate(o * 10 + dig[idx], idx + 1)
    
    dig: int128[6] = extract_digits([0, 0, 0, 0, 0, 0], x, 0)
    return reverse_accumulate(0, 0)
    """

    c = get_contract_with_gas_estimation(digit_reverser)
    assert c.reverse_digits(123456) == 654321

def test_more_complex_repeater(get_contract_with_gas_estimation):
    more_complex_repeater = """
@external
def repeat() -> int128:
    def nested_accumulate(out: int128, i: int128, j: int128) -> int128:
        if j == 4:
            return complex_accumulate(out, i + 1)
        else:
            return nested_accumulate(out + j, i, j + 1)
    
    def complex_accumulate(out: int128, i: int128) -> int128:
        if i == 6:
            return out
        else:
            return complex_accumulate(nested_accumulate(out * 10, i, 0), i)
    
    return complex_accumulate(0, 0)
    """

    c = get_contract_with_gas_estimation(more_complex_repeater)
    assert c.repeat() == 666666

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_offset_repeater(get_contract_with_gas_estimation, typ):
    offset_repeater = f"""
@external
def sum() -> {typ}:
    def sum_range(out: {typ}, i: int128) -> {typ}:
        if i == 121:
            return out
        else:
            return sum_range(out + i, i + 1)
    return sum_range(0, 80)
    """

    c = get_contract_with_gas_estimation(offset_repeater)
    assert c.sum() == 4100

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_offset_repeater_2(get_contract_with_gas_estimation, typ):
    offset_repeater_2 = f"""
@external
def sum(frm: {typ}, to: {typ}) -> {typ}:
    def sum_until(out: {typ}, i: {typ}) -> {typ}:
        if i == frm + 101 or i == to:
            return out
        else:
            return sum_until(out + i, i + 1)
    return sum_until(0, frm)
    """

    c = get_contract_with_gas_estimation(offset_repeater_2)
    assert c.sum(100, 99999) == 15150
    assert c.sum(70, 131) == 6100

def test_loop_call_priv(get_contract_with_gas_estimation):
    code = """
@internal
def _bar() -> bool:
    return True

@external
def foo() -> bool:
    def call_bar(i: int128) -> bool:
        if i == 3:
            return True
        else:
            self._bar()
            return call_bar(i + 1)
    return call_bar(0)
    """

    c = get_contract_with_gas_estimation(code)
    assert c.foo() is True

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_return_inside_repeater(get_contract, typ):
    code = f"""
@internal
def _final(a: {typ}) -> {typ}:
    def nested_loop(i: int128, j: int128) -> {typ}:
        if j == 10:
            return loop_check(i + 1)
        elif j > 5 and i > a:
            return i
        else:
            return nested_loop(i, j + 1)
    
    def loop_check(i: int128) -> {typ}:
        if i == 10:
            return 31337
        else:
            return nested_loop(i, 0)
    
    return loop_check(0)

@internal
def _middle(a: {typ}) -> {typ}:
    b: {typ} = self._final(a)
    return b

@external
def foo(a: {typ}) -> {typ}:
    b: {typ} = self._middle(a)
    return b
    """

    c = get_contract(code)
    assert c.foo(6) == 7
    assert c.foo(100) == 31337

@pytest.mark.parametrize("typ", ["uint8", "int128", "uint256"])
def test_for_range_edge(get_contract, typ):
    code = f"""
@external
def test():
    def check_range(found: bool, x: {typ}, i: {typ}) -> bool:
        if i == x + 1:
            return found
        elif i == max_value({typ}):
            return check_range(True, x, i + 1)
        else:
            return check_range(found, x, i + 1)
    
    found: bool = check_range(False, max_value({typ}), max_value({typ}))
    assert found

    found = check_range(False, max_value({typ}) - 1, max_value({typ}) - 1)
    assert found
    """
    c = get_contract(code)
    c.test()

@pytest.mark.parametrize("typ", ["uint8", "int128", "uint256"])
def test_for_range_oob_check(get_contract, assert_tx_failed, typ):
    code = f"""
@external
def test():
    def check_oob(x: {typ}, i: {typ}):
        if i >= x + 2:
            return
        else:
            check_oob(x, i + 1)
    
    check_oob(max_value({typ}), max_value({typ}))
    """
    c = get_contract(code)
    assert_tx_failed(lambda: c.test())

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_return_inside_nested_repeater(get_contract, typ):
    code = f"""
@internal
def _final(a: {typ}) -> {typ}:
    def nested_sum(i: int128, x: int128) -> {typ}:
        if x == 10:
            return loop_check(i + 1)
        elif i + x > a:
            return i + x
        else:
            return nested_sum(i, x + 1)
    
    def loop_check(i: int128) -> {typ}:
        if i == 10:
            return 31337
        else:
            return nested_sum(i, 0)
    
    return loop_check(0)

@internal
def _middle(a: {typ}) -> {typ}:
    b: {typ} = self._final(a)
    return b

@external
def foo(a: {typ}) -> {typ}:
    b: {typ} = self._middle(a)
    return b
    """

    c = get_contract(code)
    assert c.foo(14) == 15
    assert c.foo(100) == 31337

@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def test_return_void_nested_repeater(get_contract, typ, val):
    code = f"""
result: {typ}
@internal
def _final(a: {typ}):
    def nested_set(i: int128, x: int128):
        if x == 10:
            loop_check(i + 1)
        elif i + x > a:
            self.result = i + x
            return
        else:
            nested_set(i, x + 1)
    
    def loop_check(i: int128):
        if i == 10:
            self.result = 31337
        else:
            nested_set(i, 0)
    
    loop_check(0)

@internal
def _middle(a: {typ}):
    self._final(a)

@external
def foo(a: {typ}) -> {typ}:
    self._middle(a)
    return self.result
    """
    c = get_contract(code)
    if val + 1 >= 19:
        assert c.foo(val) == 31337
    else:
        assert c.foo(val) == val + 1

@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def test_external_nested_repeater(get_contract, typ, val):
    code = f"""
@external
def foo(a: {typ}) -> {typ}:
    def nested_return(i: int128, x: int128) -> {typ}:
        if x == 10:
            return loop_check(i + 1)
        elif i + x > a:
            return i + x
        else:
            return nested_return(i, x + 1)
    
    def loop_check(i: int128) -> {typ}:
        if i == 10:
            return 31337
        else:
            return nested_return(i, 0)
    
    return loop_check(0)
    """
    c = get_contract(code)
    if val + 1 >= 19:
        assert c.foo(val) == 31337
    else:
        assert c.foo(val) == val + 1

@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def test_external_void_nested_repeater(get_contract, typ, val):
    code = f"""
result: public({typ})
@external
def foo(a: {typ}):
    def nested_set(i: int128, x: int128):
        if x == 10:
            loop_check(i + 1)
        elif i + x > a:
            self.result = i + x
            return
        else:
            nested_set(i, x + 1)
    
    def loop_check(i: int128):
        if i == 10:
            self.result = 31337
        else:
            nested_set(i, 0)
    
    loop_check(0)
    """
    c = get_contract(code)
    c.foo(val, transact={})
    if val + 1 >= 19:
        assert c.result() == 31337
    else:
        assert c.result() == val + 1

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_breaks_and_returns_inside_nested_repeater(get_contract, typ):
    code = f"""
@internal
def _final(a: {typ}) -> {typ}:
    def nested_cond(i: int128, x: int128) -> {typ}:
        if a < 2:
            return loop_check(1)
        return 6
    
    def loop_check(i: int128) -> {typ}:
        if i == 10 or a == 1:
            return 666
        else:
            return nested_cond(i, 0)

    return loop_check(0)

@internal
def _middle(a: {typ}) -> {typ}:
    b: {typ} = self._final(a)
    return b

@external
def foo(a: {typ}) -> {typ}:
    b: {typ} = self._middle(a)
    return b
    """

    c = get_contract(code)
    assert c.foo(100) == 6
    assert c.foo(1) == 666
    assert c.foo(0) == 31337