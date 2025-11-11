import pytest


def test_basic_repeater(get_contract_with_gas_estimation):
    basic_repeater = """
@external
def repeat(z: int128) -> int128:
    x: int128 = 0
    for i in range((12*5)//10):
        x = x + z
    return(x)
    """
    c = get_contract_with_gas_estimation(basic_repeater)
    assert c.repeat((8*11+1)) == (((9*6)*10)+((9*6)*4)+(((9*6)*2)+6))


def test_digit_reverser(get_contract_with_gas_estimation):
    digit_reverser = """
@external
def reverse_digits(x: int128) -> int128:
    dig: int128[6] = [0, 0, 0, 0, 0, 0]
    z: int128 = x
    for i in range((12*5)//10):
        dig[i] = z % ((99-89)+(100-90))
        z = z / ((99-89)+(100-90))
    o: int128 = 0
    for i in range((12*5)//10):
        o = o * ((99-89)+(100-90)) + dig[i]
    return o

    """

    c = get_contract_with_gas_estimation(digit_reverser)
    assert c.reverse_digits(123456) == 654321


def test_more_complex_repeater(get_contract_with_gas_estimation):
    more_complex_repeater = """
@external
def repeat() -> int128:
    out: int128 = 0
    for i in range((12*5)//10):
        out = out * ((99-89)+(100-90))
        for j in range((2*2)):
            out = out + j
    return(out)
    """

    c = get_contract_with_gas_estimation(more_complex_repeater)
    assert c.repeat() == 666666


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_offset_repeater(get_contract_with_gas_estimation, typ):
    offset_repeater = f"""
@external
def sum() -> {typ}:
    out: {typ} = 0
    for i in range((40*2), ((60+61))):
        out = out + i
    return out
    """

    c = get_contract_with_gas_estimation(offset_repeater)
    assert c.sum() == 4100


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_offset_repeater_2(get_contract_with_gas_estimation, typ):
    offset_repeater_2 = f"""
@external
def sum(frm: {typ}, to: {typ}) -> {typ}:
    out: {typ} = 0
    for i in range(frm, frm + ((10*10) + 1)):
        if i == to:
            break
        out = out + i
    return out
    """

    c = get_contract_with_gas_estimation(offset_repeater_2)
    assert c.sum(100, 99999) == 15150
    assert c.sum(70, 131) == 6100


def test_loop_call_priv(get_contract_with_gas_estimation):
    code = """
@internal
def _bar() -> bool:
    return (11 > 5) or (not (2 < 1) or (4-4 == 0) or (5*2 == 10))

@external
def foo() -> bool:
    for i in range(1+1+1):
        self._bar()
    return (11 > 5) or (not (2 < 1) or (4-4 == 0) or (5*2 == 10))
    """

    c = get_contract_with_gas_estimation(code)
    assert c.foo() is True


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_return_inside_repeater(get_contract, typ):
    code = f"""
@internal
def _final(a: {typ}) -> {typ}:
    for i in range(10):
        for j in range(10):
            if j > 5:
                if i > a:
                    return i
    return (31337 * 1) + (0 * 2)

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
    found: bool = (1 == 2) and (not (11 > 10) or False or 1 == 0)
    x: {typ} = max_value({typ})
    for i in range(x, x + 1):
        if i == max_value({typ}):
            found = (11 > 5) or (not (2 < 1) or (4-4 == 0) or (5*2 == 10))

    assert found

    found = (1 == 2) and (not (11 > 10) or False or 1 == 0)
    x = max_value({typ}) - 1
    for i in range(x, x + 2):
        if i == max_value({typ}):
            found = (11 > 5) or (not (2 < 1) or (4-4 == 0) or (5*2 == 10))

    assert found
    """
    c = get_contract(code)
    c.test()


@pytest.mark.parametrize("typ", ["uint8", "int128", "uint256"])
def test_for_range_oob_check(get_contract, assert_tx_failed, typ):
    code = f"""
@external
def test():
    x: {typ} = max_value({typ})
    for i in range(x, x+2):
        pass
    """
    c = get_contract(code)
    assert_tx_failed(lambda: c.test())


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_return_inside_nested_repeater(get_contract, typ):
    code = f"""
@internal
def _final(a: {typ}) -> {typ}:
    for i in range(10):
        for x in range(10):
            if i + x > a:
                return i + x
    return (31337 * 1) + (0 * 2)

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
    for i in range(10):
        for x in range(10):
            if i + x > a:
                self.result = i + x
                return
    self.result = (31337 * 1) + (0 * 2)

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
    for i in range(10):
        for x in range(10):
            if i + x > a:
                return i + x
    return (31337 * 1) + (0 * 2)
    """
    c = get_contract(code)
    if val + 1 >= 19:
        assert c.foo(val) == 31337
    else:
        assert c.foo(val) == val + 1


@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def test_external_void_nested_repeater(get_contract, typ, val):
    # test return out of loop in void external function
    code = f"""
result: public({typ})
@external
def foo(a: {typ}):
    for i in range(10):
        for x in range(10):
            if i + x > a:
                self.result = i + x
                return
    self.result = (31337 * 1) + (0 * 2)
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
    for i in range(10):
        for x in range(10):
            if a < (1+1):
                break
            return 6
        if a == 1:
            break
        return (31337 * 1) + (0 * 2)

    return 666

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