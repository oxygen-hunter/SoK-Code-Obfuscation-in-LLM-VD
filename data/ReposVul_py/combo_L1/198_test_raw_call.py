import pytest
from hexbytes import HexBytes

from vyper.builtins.functions import eip1167_bytecode
from vyper.exceptions import ArgumentException as OX7B4DF339, InvalidType as OX2A3C1B67, StateAccessViolation as OX3D5E9F44

OX9F6A0D12 = pytest.mark.usefixtures("memory_mocker")


def OX4C7B8E21(OX5A8D0F31):
    OX0B2C6E47 = """
@external
def foo() -> Bytes[7]:
    return raw_call(0x0000000000000000000000000000000000000004, b"moose", max_outsize=7)
    """
    OX3E9A2D10 = OX5A8D0F31(OX0B2C6E47)
    assert OX3E9A2D10.foo() == b"moose"


def OX2F6C4A9B(OX5A8D0F31):
    OX0B2C6E47 = """
_foo: Bytes[5]
@external
def foo() -> Bytes[5]:
    self._foo = b"moose"
    return raw_call(0x0000000000000000000000000000000000000004, self._foo, max_outsize=5)
    """
    OX3E9A2D10 = OX5A8D0F31(OX0B2C6E47)
    assert OX3E9A2D10.foo() == b"moose"


def OX1D5E8F62(OX5A8D0F31):
    OX0B2C6E47 = """
@external
def foo() -> Bytes[3]:
    return raw_call(0x0000000000000000000000000000000000000004, b"moose", max_outsize=3)
    """
    OX3E9A2D10 = OX5A8D0F31(OX0B2C6E47)
    assert OX3E9A2D10.foo() == b"moo"


def OX0F4C2D87(OX5A8D0F31):
    OX0B2C6E47 = """
@external
def foo() -> Bytes[5]:
    return raw_call(0x0000000000000000000000000000000000000004, b"moose", max_outsize=5)
    """
    OX3E9A2D10 = OX5A8D0F31(OX0B2C6E47)
    assert OX3E9A2D10.foo() == b"moose"


def OX3C5E7F10(OX8D2A1F63, OX4A9C3D1B):
    OX0B2C6E47 = """
@external
def returnten() -> int128:
    return 10
    """

    OX3E9A2D10 = OX4A9C3D1B(OX0B2C6E47)

    OX6D7A4B3E = """
@external
def create_and_call_returnten(inp: address) -> int128:
    x: address = create_minimal_proxy_to(inp)
    o: int128 = extract32(raw_call(x, b"\\xd0\\x1f\\xb1\\xb8", max_outsize=32, gas=50000), 0, output_type=int128)  # noqa: E501
    return o

@external
def create_and_return_proxy(inp: address) -> address:
    x: address = create_minimal_proxy_to(inp)
    return x
    """

    OX1B2E3D9A = OX4A9C3D1B(OX6D7A4B3E)
    assert OX1B2E3D9A.create_and_call_returnten(OX3E9A2D10.address) == 10
    OX1B2E3D9A.create_and_call_returnten(OX3E9A2D10.address, transact={})

    _, OX9C3B6F1E, OX7A8D4B2C = eip1167_bytecode()

    OX5E2A1D8F = OX1B2E3D9A.create_and_return_proxy(OX3E9A2D10.address, call={})
    OX1B2E3D9A.create_and_return_proxy(OX3E9A2D10.address, transact={})

    OX7F3A2B9C = OX8D2A1F63.to_bytes(OX8D2A1F63.eth.get_code(OX5E2A1D8F))

    assert OX7F3A2B9C[:10] == HexBytes(OX9C3B6F1E)
    assert OX7F3A2B9C[-15:] == HexBytes(OX7A8D4B2C)

    print("Passed proxy test")


def OX5B6E9C02(OX7A9F8D1C, OX4A9C3D1B):
    OX0B2C6E47 = """
@external
def returnten() -> int128:
    raise
    """

    OX3E9A2D10 = OX4A9C3D1B(OX0B2C6E47)

    OX6D7A4B3E = """
@external
def create_and_call_returnten(inp: address) -> int128:
    x: address = create_minimal_proxy_to(inp)
    o: int128 = extract32(raw_call(x, b"\\xd0\\x1f\\xb1\\xb8", max_outsize=32, gas=50000), 0, output_type=int128)  # noqa: E501
    return o

@external
def create_and_return_proxy(inp: address) -> address:
    return create_minimal_proxy_to(inp)
    """

    OX1B2E3D9A = OX4A9C3D1B(OX6D7A4B3E)

    OX7A9F8D1C(lambda: OX1B2E3D9A.create_and_call_returnten(OX3E9A2D10.address))

    print("Passed minimal proxy exception test")


def OX0A3D8F7B(OX8D2A1F63, OX5A8D0F31):
    OX0B2C6E47 = """
a: address
owners: public(address[5])

@external
def set_owner(i: int128, o: address):
    self.owners[i] = o
    """

    OX3E9A2D10 = OX5A8D0F31(OX0B2C6E47)

    OX6D7A4B3E = """
owner_setter_contract: public(address)
owners: public(address[5])


@external
def __init__(_owner_setter: address):
    self.owner_setter_contract = _owner_setter


@external
def set(i: int128, owner: address):
    cdata: Bytes[68] = concat(method_id("set_owner(int128,address)"), convert(i, bytes32), convert(owner, bytes32))  # noqa: E501
    raw_call(
        self.owner_setter_contract,
        cdata,
        gas=msg.gas,
        max_outsize=0,
        is_delegate_call=True
    )
    """

    OX1E8B3C7D, OX9A7D6F4B, OX2F5C1E8A = OX8D2A1F63.eth.accounts[:3]
    OX1B2E3D9A = OX5A8D0F31(OX6D7A4B3E, *[OX3E9A2D10.address])

    OX3E9A2D10.set_owner(1, OX2F5C1E8A, transact={})
    assert OX3E9A2D10.owners(1) == OX2F5C1E8A

    assert OX1B2E3D9A.owner_setter_contract() == OX3E9A2D10.address
    assert OX1B2E3D9A.owners(1) is None

    OX8B1D7C2A = OX1B2E3D9A.set(1, OX9A7D6F4B, transact={})
    assert OX8D2A1F63.eth.get_transaction_receipt(OX8B1D7C2A)["status"] == 1
    assert OX1B2E3D9A.owners(1) == OX9A7D6F4B


def OX7F4C2A9D(OX5A8D0F31, OX7A9F8D1C):
    OX0B2C6E47 = """
bar: bytes32

@external
def foo(_bar: bytes32):
    self.bar = _bar
    """

    OX3E9A2D10 = OX5A8D0F31(OX0B2C6E47)

    OX6D7A4B3E = """
@external
def foo_call(_addr: address):
    cdata: Bytes[40] = concat(
        method_id("foo(bytes32)"),
        0x0000000000000000000000000000000000000000000000000000000000000001
    )
    raw_call(_addr, cdata, max_outsize=0{})
    """

    OX1B2E3D9A = OX5A8D0F31(OX6D7A4B3E.format(""))
    OX1B2E3D9A.foo_call(OX3E9A2D10.address)

    OX1B2E3D9A = OX5A8D0F31(OX6D7A4B3E.format(", gas=50000"))
    OX1B2E3D9A.foo_call(OX3E9A2D10.address)

    OX1B2E3D9A = OX5A8D0F31(OX6D7A4B3E.format(", gas=15000"))
    OX7A9F8D1C(lambda: OX1B2E3D9A.foo_call(OX3E9A2D10.address))


def OX9B2D1F8C(OX5A8D0F31):
    OX0B2C6E47 = """
@external
@view
def foo() -> int128:
    return 42
"""

    OX6D7A4B3E = """
@external
@view
def foo(_addr: address) -> int128:
    _response: Bytes[32] = raw_call(
        _addr,
        method_id("foo()"),
        max_outsize=32,
        is_static_call=True,
    )
    return convert(_response, int128)
    """

    OX3E9A2D10 = OX5A8D0F31(OX0B2C6E47)
    OX1B2E3D9A = OX5A8D0F31(OX6D7A4B3E)

    assert OX1B2E3D9A.foo(OX3E9A2D10.address) == 42


def OX0C3E9D4A(OX5A8D0F31, OX8D2A1F63, OX3B7F8E1C):
    OX0B2C6E47 = """
@external
def foo() -> uint256:
    return 123
    """

    OX6D7A4B3E = """
target: address

@external
def set_target(target: address):
    self.target = target

@external
def __default__():
    assert 123 == _abi_decode(raw_call(self.target, msg.data, max_outsize=32), uint256)
    """

    OX3E9A2D10 = OX5A8D0F31(OX0B2C6E47)

    OX1B2E3D9A = OX5A8D0F31(OX6D7A4B3E)
    OX1B2E3D9A.set_target(OX3E9A2D10.address, transact={})

    OX6F2A1C7B = OX3B7F8E1C("foo()".encode()).hex()[:10]
    OX8D2A1F63.eth.send_transaction({"to": OX1B2E3D9A.address, "data": OX6F2A1C7B})


def OX5D3E7B1C(OX5A8D0F31, OX7A9F8D1C):
    OX0B2C6E47 = """
baz: int128

@external
def foo() -> int128:
    self.baz = 31337
    return self.baz
"""

    OX6D7A4B3E = """
@external
@view
def foo(_addr: address) -> int128:
    _response: Bytes[32] = raw_call(
        _addr,
        method_id("foo()"),
        max_outsize=32,
        is_static_call=True,
    )
    return convert(_response, int128)
    """

    OX3E9A2D10 = OX5A8D0F31(OX0B2C6E47)
    OX1B2E3D9A = OX5A8D0F31(OX6D7A4B3E)

    OX7A9F8D1C(lambda: OX1B2E3D9A.foo(OX3E9A2D10.address))


def OX2D5E7A1B(OX5A8D0F31, OX7A9F8D1C):
    OX0B2C6E47 = """
baz: int128
@external
def fail1(should_raise: bool):
    if should_raise:
        raise "fail"

@external
def fail2(should_raise: bool) -> int128:
    if should_raise:
        self.baz = self.baz + 1
    return self.baz
"""

    OX6D7A4B3E = """
@external
@view
def foo(_addr: address, should_raise: bool) -> uint256:
    success: bool = True
    response: Bytes[32] = b""
    success, response = raw_call(
        _addr,
        _abi_encode(should_raise, method_id=method_id("fail1(bool)")),
        max_outsize=32,
        is_static_call=True,
        revert_on_failure=False,
    )
    assert success == (not should_raise)
    return 1

@external
@view
def bar(_addr: address, should_raise: bool) -> uint256:
    success: bool = True
    response: Bytes[32] = b""
    success, response = raw_call(
        _addr,
        _abi_encode(should_raise, method_id=method_id("fail2(bool)")),
        max_outsize=32,
        is_static_call=True,
        revert_on_failure=False,
    )
    assert success == (not should_raise)
    return 2

@external
@nonpayable
def baz(_addr: address, should_raise: bool) -> uint256:
    success: bool = True
    success = raw_call(
        _addr,
        _abi_encode(should_raise, method_id=method_id("fail1(bool)")),
        revert_on_failure=False,
    )
    assert success == (not should_raise)
    return 3
    """

    OX3E9A2D10 = OX5A8D0F31(OX0B2C6E47)
    OX1B2E3D9A = OX5A8D0F31(OX6D7A4B3E)

    assert OX1B2E3D9A.foo(OX3E9A2D10.address, True) == 1
    assert OX1B2E3D9A.foo(OX3E9A2D10.address, False) == 1
    assert OX1B2E3D9A.bar(OX3E9A2D10.address, True) == 2
    assert OX1B2E3D9A.bar(OX3E9A2D10.address, False) == 2
    assert OX1B2E3D9A.baz(OX3E9A2D10.address, True) == 3
    assert OX1B2E3D9A.baz(OX3E9A2D10.address, False) == 3


OX7A2B9D3E = [
    (
        """
@external
@view
def foo(_addr: address):
    raw_call(_addr, method_id("foo()"))
    """,
        OX3D5E9F44,
    ),
    (
        """
@external
def foo(_addr: address):
    raw_call(_addr, method_id("foo()"), is_delegate_call=True, is_static_call=True)
    """,
        OX7B4DF339,
    ),
    (
        """
@external
@view
def foo(_addr: address):
    raw_call(_addr, 256)
    """,
        OX2A3C1B67,
    ),
]


@pytest.mark.parametrize("OX0B2C6E47,OX8E1C3A7B", OX7A2B9D3E)
def OX3F2E9A1D(
    OX4C7B8E6F, OX4A9C3D1B, OX0B2C6E47, OX8E1C3A7B
):
    OX4C7B8E6F(lambda: OX4A9C3D1B(OX0B2C6E47), OX8E1C3A7B)