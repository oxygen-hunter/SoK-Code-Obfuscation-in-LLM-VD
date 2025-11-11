import pytest

def test_basic_repeater(get_contract_with_gas_estimation):
    basic_repeater = """
@external
def repeat(z: int128) -> int128:
    v: int128[2] = [0, 0]
    for i in range(6):
        v[0] = v[0] + z
    return(v[0])
    """
    c = get_contract_with_gas_estimation(basic_repeater)
    assert c.repeat(9) == 54


def test_digit_reverser(get_contract_with_gas_estimation):
    digit_reverser = """
@external
def reverse_digits(x: int128) -> int128:
    a: int128 = x
    b: int128[6] = [0, 0, 0, 0, 0, 0]
    for i in range(6):
        b[i] = a % 10
        a = a / 10
    c: int128 = 0
    for i in range(6):
        c = c * 10 + b[i]
    return c

    """

    c = get_contract_with_gas_estimation(digit_reverser)
    assert c.reverse_digits(123456) == 654321


def test_more_complex_repeater(get_contract_with_gas_estimation):
    more_complex_repeater = """
@external
def repeat() -> int128:
    v: int128[2] = [0, 0]
    for i in range(6):
        v[0] = v[0] * 10
        for j in range(4):
            v[0] = v[0] + j
    return(v[0])
    """

    c = get_contract_with_gas_estimation(more_complex_repeater)
    assert c.repeat() == 666666


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_offset_repeater(get_contract_with_gas_estimation, typ):
    offset_repeater = f"""
@external
def sum() -> {typ}:
    v: {typ}[2] = [0, 0]
    for i in range(80, 121):
        v[0] = v[0] + i
    return v[0]
    """

    c = get_contract_with_gas_estimation(offset_repeater)
    assert c.sum() == 4100


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_offset_repeater_2(get_contract_with_gas_estimation, typ):
    offset_repeater_2 = f"""
@external
def sum(frm: {typ}, to: {typ}) -> {typ}:
    v: {typ}[2] = [0, 0]
    for i in range(frm, frm + 101):
        if i == to:
            break
        v[0] = v[0] + i
    return v[0]
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
    for i in range(3):
        self._bar()
    return True
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
    return 31337

@internal
def _middle(a: {typ}) -> {typ}:
    return self._final(a)

@external
def foo(a: {typ}) -> {typ}:
    return self._middle(a)
    """

    c = get_contract(code)
    assert c.foo(6) == 7
    assert c.foo(100) == 31337


@pytest.mark.parametrize("typ", ["uint8", "int128", "uint256"])
def test_for_range_edge(get_contract, typ):
    code = f"""
@external
def test():
    v: {typ}[2] = [False, max_value({typ})]
    for i in range(v[1], v[1] + 1):
        if i == max_value({typ}):
            v[0] = True

    assert v[0]

    v[0] = False
    v[1] = max_value({typ}) - 1
    for i in range(v[1], v[1] + 2):
        if i == max_value({typ}):
            v[0] = True

    assert v[0]
    """
    c = get_contract(code)
    c.test()


@pytest.mark.parametrize("typ", ["uint8", "int128", "uint256"])
def test_for_range_oob_check(get_contract, assert_tx_failed, typ):
    code = f"""
@external
def test():
    v: {typ}[1] = [max_value({typ})]
    for i in range(v[0], v[0] + 2):
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
    return 31337

@internal
def _middle(a: {typ}) -> {typ}:
    return self._final(a)

@external
def foo(a: {typ}) -> {typ}:
    return self._middle(a)
    """

    c = get_contract(code)
    assert c.foo(14) == 15
    assert c.foo(100) == 31337


@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def test_return_void_nested_repeater(get_contract, typ, val):
    code = f"""
v: {typ}
@internal
def _final(a: {typ}):
    for i in range(10):
        for x in range(10):
            if i + x > a:
                v = i + x
                return
    v = 31337

@internal
def _middle(a: {typ}):
    self._final(a)

@external
def foo(a: {typ}) -> {typ}:
    self._middle(a)
    return v
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
    return 31337
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
v: public({typ})
@external
def foo(a: {typ}):
    for i in range(10):
        for x in range(10):
            if i + x > a:
                v = i + x
                return
    v = 31337
    """
    c = get_contract(code)
    c.foo(val, transact={})
    if val + 1 >= 19:
        assert c.v() == 31337
    else:
        assert c.v() == val + 1


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_breaks_and_returns_inside_nested_repeater(get_contract, typ):
    code = f"""
@internal
def _final(a: {typ}) -> {typ}:
    for i in range(10):
        for x in range(10):
            if a < 2:
                break
            return 6
        if a == 1:
            break
        return 31337

    return 666

@internal
def _middle(a: {typ}) -> {typ}:
    return self._final(a)

@external
def foo(a: {typ}) -> {typ}:
    return self._middle(a)
    """

    c = get_contract(code)
    assert c.foo(100) == 6
    assert c.foo(1) == 666
    assert c.foo(0) == 31337