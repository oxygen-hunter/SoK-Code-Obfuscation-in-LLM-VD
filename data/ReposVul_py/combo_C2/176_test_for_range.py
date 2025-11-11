import pytest

def test_basic_repeater(get_contract_with_gas_estimation):
    basic_repeater = """
@external
def repeat(z: int128) -> int128:
    x: int128 = 0
    _state: int128 = 0
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            _state = 0
            _control = 1
        elif _control == 1:
            if _state < 6:
                x = x + z
                _state += 1
                _control = 1
            else:
                _control = 2
        elif _control == 2:
            return x
    """
    c = get_contract_with_gas_estimation(basic_repeater)
    assert c.repeat(9) == 54

def test_digit_reverser(get_contract_with_gas_estimation):
    digit_reverser = """
@external
def reverse_digits(x: int128) -> int128:
    dig: int128[6] = [0, 0, 0, 0, 0, 0]
    z: int128 = x
    _state: int128 = 0
    _control: int128 = 0
    o: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state < 6:
                dig[_state] = z % 10
                z = z / 10
                _state += 1
                _control = 0
            else:
                _state = 0
                _control = 1
        elif _control == 1:
            if _state < 6:
                o = o * 10 + dig[_state]
                _state += 1
                _control = 1
            else:
                return o
    """
    c = get_contract_with_gas_estimation(digit_reverser)
    assert c.reverse_digits(123456) == 654321

def test_more_complex_repeater(get_contract_with_gas_estimation):
    more_complex_repeater = """
@external
def repeat() -> int128:
    out: int128 = 0
    _state_i: int128 = 0
    _state_j: int128 = 0
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state_i < 6:
                out = out * 10
                _state_j = 0
                _control = 1
            else:
                _control = 2
        elif _control == 1:
            if _state_j < 4:
                out = out + _state_j
                _state_j += 1
                _control = 1
            else:
                _state_i += 1
                _control = 0
        elif _control == 2:
            return out
    """
    c = get_contract_with_gas_estimation(more_complex_repeater)
    assert c.repeat() == 666666

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_offset_repeater(get_contract_with_gas_estimation, typ):
    offset_repeater = f"""
@external
def sum() -> {typ}:
    out: {typ} = 0
    _state: int128 = 80
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state < 121:
                out = out + _state
                _state += 1
                _control = 0
            else:
                _control = 1
        elif _control == 1:
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
    _state: {typ} = frm
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state == to:
                _control = 2
            elif _state < frm + 101:
                out = out + _state
                _state += 1
                _control = 0
            else:
                _control = 1
        elif _control == 1:
            return out
        elif _control == 2:
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
    _state: int128 = 0
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state < 3:
                self._bar()
                _state += 1
                _control = 0
            else:
                return True
    """
    c = get_contract_with_gas_estimation(code)
    assert c.foo() is True

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_return_inside_repeater(get_contract, typ):
    code = f"""
@internal
def _final(a: {typ}) -> {typ}:
    _state_i: int128 = 0
    _state_j: int128 = 0
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state_i < 10:
                _state_j = 0
                _control = 1
            else:
                return 31337
        elif _control == 1:
            if _state_j < 10:
                if _state_j > 5:
                    if _state_i > a:
                        return _state_i
                _state_j += 1
                _control = 1
            else:
                _state_i += 1
                _control = 0
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
    assert c.foo(6) == 7
    assert c.foo(100) == 31337

@pytest.mark.parametrize("typ", ["uint8", "int128", "uint256"])
def test_for_range_edge(get_contract, typ):
    code = f"""
@external
def test():
    found: bool = False
    x: {typ} = max_value({typ})
    _state: {typ} = x
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state < x + 1:
                if _state == max_value({typ}):
                    found = True
                _state += 1
                _control = 0
            else:
                assert found
                found = False
                x = max_value({typ}) - 1
                _state = x
                _control = 1
        elif _control == 1:
            if _state < x + 2:
                if _state == max_value({typ}):
                    found = True
                _state += 1
                _control = 1
            else:
                assert found
                _control = -1
    """
    c = get_contract(code)
    c.test()

@pytest.mark.parametrize("typ", ["uint8", "int128", "uint256"])
def test_for_range_oob_check(get_contract, assert_tx_failed, typ):
    code = f"""
@external
def test():
    x: {typ} = max_value({typ})
    _state: {typ} = x
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state < x + 2:
                _state += 1
                _control = 0
            else:
                _control = -1
    """
    c = get_contract(code)
    assert_tx_failed(lambda: c.test())

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_return_inside_nested_repeater(get_contract, typ):
    code = f"""
@internal
def _final(a: {typ}) -> {typ}:
    _state_i: int128 = 0
    _state_x: int128 = 0
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state_i < 10:
                _state_x = 0
                _control = 1
            else:
                return 31337
        elif _control == 1:
            if _state_x < 10:
                if _state_i + _state_x > a:
                    return _state_i + _state_x
                _state_x += 1
                _control = 1
            else:
                _state_i += 1
                _control = 0
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
    assert c.foo(14) == 15
    assert c.foo(100) == 31337

@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def test_return_void_nested_repeater(get_contract, typ, val):
    code = f"""
result: {typ}
@internal
def _final(a: {typ}):
    _state_i: int128 = 0
    _state_x: int128 = 0
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state_i < 10:
                _state_x = 0
                _control = 1
            else:
                self.result = 31337
                return
        elif _control == 1:
            if _state_x < 10:
                if _state_i + _state_x > a:
                    self.result = _state_i + _state_x
                    return
                _state_x += 1
                _control = 1
            else:
                _state_i += 1
                _control = 0
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
    _state_i: int128 = 0
    _state_x: int128 = 0
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state_i < 10:
                _state_x = 0
                _control = 1
            else:
                return 31337
        elif _control == 1:
            if _state_x < 10:
                if _state_i + _state_x > a:
                    return _state_i + _state_x
                _state_x += 1
                _control = 1
            else:
                _state_i += 1
                _control = 0
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
result: public({typ})
@external
def foo(a: {typ}):
    _state_i: int128 = 0
    _state_x: int128 = 0
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state_i < 10:
                _state_x = 0
                _control = 1
            else:
                self.result = 31337
                return
        elif _control == 1:
            if _state_x < 10:
                if _state_i + _state_x > a:
                    self.result = _state_i + _state_x
                    return
                _state_x += 1
                _control = 1
            else:
                _state_i += 1
                _control = 0
    self.result = 31337
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
    _state_i: int128 = 0
    _state_x: int128 = 0
    _control: int128 = 0
    while _control != -1:
        if _control == 0:
            if _state_i < 10:
                _state_x = 0
                _control = 1
            else:
                return 666
        elif _control == 1:
            if _state_x < 10:
                if a < 2:
                    _control = 3
                else:
                    return 6
                _state_x += 1
                _control = 1
            else:
                if a == 1:
                    _control = 3
                else:
                    return 31337
                _state_i += 1
                _control = 0
        elif _control == 3:
            return 666
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