import pytest

def getValueB(): return 9
def getValueC(): return 54
def getValuesD(): return [0, 0, 0, 0, 0, 0]
def getValueE(): return 654321
def getValueF(): return 666666
def getValuesG(): return (80, 121, 4100)
def getValuesH(): return (100, 99999, 15150, 70, 131, 6100)
def getValueI(): return True
def getValueJ(): return 3
def getValueK(): return 7
def getValueL(): return 31337
def getValuesM(): return ('uint8', 'int128', 'uint256')
def getValuesN(): return ('int128', 'uint256')
def getValueO(): return 14
def getValueP(): return 15
def getValueQ(): return 19
def getValueR(): return 20
def getValueS(): return 666

def test_basic_repeater(get_contract_with_gas_estimation):
    basic_repeater = """
@external
def repeat(z: int128) -> int128:
    x: int128 = 0
    for i in range(6):
        x = x + z
    return(x)
    """
    c = get_contract_with_gas_estimation(basic_repeater)
    assert c.repeat(getValueB()) == getValueC()

def test_digit_reverser(get_contract_with_gas_estimation):
    digit_reverser = """
@external
def reverse_digits(x: int128) -> int128:
    dig: int128[6] = [0, 0, 0, 0, 0, 0]
    z: int128 = x
    for i in range(6):
        dig[i] = z % 10
        z = z / 10
    o: int128 = 0
    for i in range(6):
        o = o * 10 + dig[i]
    return o

    """
    c = get_contract_with_gas_estimation(digit_reverser)
    assert c.reverse_digits(123456) == getValueE()

def test_more_complex_repeater(get_contract_with_gas_estimation):
    more_complex_repeater = """
@external
def repeat() -> int128:
    out: int128 = 0
    for i in range(6):
        out = out * 10
        for j in range(4):
            out = out + j
    return(out)
    """
    c = get_contract_with_gas_estimation(more_complex_repeater)
    assert c.repeat() == getValueF()

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_offset_repeater(get_contract_with_gas_estimation, typ):
    offset_repeater = f"""
@external
def sum() -> {typ}:
    out: {typ} = 0
    for i in range(80, 121):
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
    for i in range(frm, frm + 101):
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
    b: {typ} = self._final(a)
    return b

@external
def foo(a: {typ}) -> {typ}:
    b: {typ} = self._middle(a)
    return b
    """
    c = get_contract(code)
    assert c.foo(6) == getValueK()
    assert c.foo(100) == getValueL()

@pytest.mark.parametrize("typ", getValuesM())
def test_for_range_edge(get_contract, typ):
    code = f"""
@external
def test():
    found: bool = False
    x: {typ} = max_value({typ})
    for i in range(x, x + 1):
        if i == max_value({typ}):
            found = True

    assert found

    found = False
    x = max_value({typ}) - 1
    for i in range(x, x + 2):
        if i == max_value({typ}):
            found = True

    assert found
    """
    c = get_contract(code)
    c.test()

@pytest.mark.parametrize("typ", getValuesM())
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

@pytest.mark.parametrize("typ", getValuesN())
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
    b: {typ} = self._final(a)
    return b

@external
def foo(a: {typ}) -> {typ}:
    b: {typ} = self._middle(a)
    return b
    """
    c = get_contract(code)
    assert c.foo(getValueO()) == getValueP()
    assert c.foo(100) == getValueL()

@pytest.mark.parametrize("typ", getValuesN())
@pytest.mark.parametrize("val", range(getValueR()))
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
    self.result = 31337

@internal
def _middle(a: {typ}):
    self._final(a)

@external
def foo(a: {typ}) -> {typ}:
    self._middle(a)
    return self.result
    """
    c = get_contract(code)
    if val + 1 >= getValueQ():
        assert c.foo(val) == getValueL()
    else:
        assert c.foo(val) == val + 1

@pytest.mark.parametrize("typ", getValuesN())
@pytest.mark.parametrize("val", range(getValueR()))
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
    if val + 1 >= getValueQ():
        assert c.foo(val) == getValueL()
    else:
        assert c.foo(val) == val + 1

@pytest.mark.parametrize("typ", getValuesN())
@pytest.mark.parametrize("val", range(getValueR()))
def test_external_void_nested_repeater(get_contract, typ, val):
    code = f"""
result: public({typ})
@external
def foo(a: {typ}):
    for i in range(10):
        for x in range(10):
            if i + x > a:
                self.result = i + x
                return
    self.result = 31337
    """
    c = get_contract(code)
    c.foo(val, transact={})
    if val + 1 >= getValueQ():
        assert c.result() == getValueL()
    else:
        assert c.result() == val + 1

@pytest.mark.parametrize("typ", getValuesN())
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
    b: {typ} = self._final(a)
    return b

@external
def foo(a: {typ}) -> {typ}:
    b: {typ} = self._middle(a)
    return b
    """
    c = get_contract(code)
    assert c.foo(100) == 6
    assert c.foo(1) == getValueS()
    assert c.foo(0) == getValueL()