import pytest


def OX2D3BFE9C(OX9A4BDC0F):
    OX6BB6E3D0 = """
@external
def repeat(z: int128) -> int128:
    x: int128 = 0
    for i in range(6):
        x = x + z
    return(x)
    """
    OX3A3A6B12 = OX9A4BDC0F(OX6BB6E3D0)
    assert OX3A3A6B12.repeat(9) == 54


def OX470BD79E(OX9A4BDC0F):
    OX0F8DF8F7 = """
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

    OX3A3A6B12 = OX9A4BDC0F(OX0F8DF8F7)
    assert OX3A3A6B12.reverse_digits(123456) == 654321


def OX1A7BCF2F(OX9A4BDC0F):
    OX5F8F1E0A = """
@external
def repeat() -> int128:
    out: int128 = 0
    for i in range(6):
        out = out * 10
        for j in range(4):
            out = out + j
    return(out)
    """

    OX3A3A6B12 = OX9A4BDC0F(OX5F8F1E0A)
    assert OX3A3A6B12.repeat() == 666666


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def OXD4C7F4F3(OX9A4BDC0F, typ):
    OX6E3D4EC9 = f"""
@external
def sum() -> {typ}:
    out: {typ} = 0
    for i in range(80, 121):
        out = out + i
    return out
    """

    OX3A3A6B12 = OX9A4BDC0F(OX6E3D4EC9)
    assert OX3A3A6B12.sum() == 4100


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def OXCE0F9E2B(OX9A4BDC0F, typ):
    OXA3A3A3D7 = f"""
@external
def sum(frm: {typ}, to: {typ}) -> {typ}:
    out: {typ} = 0
    for i in range(frm, frm + 101):
        if i == to:
            break
        out = out + i
    return out
    """

    OX3A3A6B12 = OX9A4BDC0F(OXA3A3A3D7)
    assert OX3A3A6B12.sum(100, 99999) == 15150
    assert OX3A3A6B12.sum(70, 131) == 6100


def OXBB7B2C0C(OX9A4BDC0F):
    OX1F4E3C8D = """
@internal
def _bar() -> bool:
    return True

@external
def foo() -> bool:
    for i in range(3):
        self._bar()
    return True
    """

    OX3A3A6B12 = OX9A4BDC0F(OX1F4E3C8D)
    assert OX3A3A6B12.foo() is True


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def OX7E7A1B5F(OX8C3D4E2B, typ):
    OX6C5D7E3A = f"""
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

    OX3A3A6B12 = OX8C3D4E2B(OX6C5D7E3A)
    assert OX3A3A6B12.foo(6) == 7
    assert OX3A3A6B12.foo(100) == 31337


@pytest.mark.parametrize("typ", ["uint8", "int128", "uint256"])
def OX8A7C8C4F(OX8C3D4E2B, typ):
    OX2C4C6B6F = f"""
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
    OX3A3A6B12 = OX8C3D4E2B(OX2C4C6B6F)
    OX3A3A6B12.test()


@pytest.mark.parametrize("typ", ["uint8", "int128", "uint256"])
def OX4B8D7F2C(OX8C3D4E2B, OXA6B7D1E3, typ):
    OX1E4C5C9A = f"""
@external
def test():
    x: {typ} = max_value({typ})
    for i in range(x, x+2):
        pass
    """
    OX3A3A6B12 = OX8C3D4E2B(OX1E4C5C9A)
    OXA6B7D1E3(lambda: OX3A3A6B12.test())


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def OX6B9A0E1F(OX8C3D4E2B, typ):
    OX8E6D1F4A = f"""
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

    OX3A3A6B12 = OX8C3D4E2B(OX8E6D1F4A)
    assert OX3A3A6B12.foo(14) == 15
    assert OX3A3A6B12.foo(100) == 31337


@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def OX9F4A3F6E(OX8C3D4E2B, typ, val):
    OX3B8C2D4E = f"""
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
    OX3A3A6B12 = OX8C3D4E2B(OX3B8C2D4E)
    if val + 1 >= 19:
        assert OX3A3A6B12.foo(val) == 31337
    else:
        assert OX3A3A6B12.foo(val) == val + 1


@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def OX5E6B7A9D(OX8C3D4E2B, typ, val):
    OX8D5B9C2A = f"""
@external
def foo(a: {typ}) -> {typ}:
    for i in range(10):
        for x in range(10):
            if i + x > a:
                return i + x
    return 31337
    """
    OX3A3A6B12 = OX8C3D4E2B(OX8D5B9C2A)
    if val + 1 >= 19:
        assert OX3A3A6B12.foo(val) == 31337
    else:
        assert OX3A3A6B12.foo(val) == val + 1


@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def OX3C4F7A8D(OX8C3D4E2B, typ, val):
    OX5A7C8B3D = f"""
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
    OX3A3A6B12 = OX8C3D4E2B(OX5A7C8B3D)
    OX3A3A6B12.foo(val, transact={})
    if val + 1 >= 19:
        assert OX3A3A6B12.result() == 31337
    else:
        assert OX3A3A6B12.result() == val + 1


@pytest.mark.parametrize("typ", ["int128", "uint256"])
def OX2E7D4B1A(OX8C3D4E2B, typ):
    OX7D8E9B3A = f"""
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

    OX3A3A6B12 = OX8C3D4E2B(OX7D8E9B3A)
    assert OX3A3A6B12.foo(100) == 6
    assert OX3A3A6B12.foo(1) == 666
    assert OX3A3A6B12.foo(0) == 31337