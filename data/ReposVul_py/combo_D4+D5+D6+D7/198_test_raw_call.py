import pytest
from hexbytes import HexBytes
from vyper.builtins.functions import eip1167_bytecode
from vyper.exceptions import ArgumentException, InvalidType, StateAccessViolation
pytestmark = pytest.mark.usefixtures("memory_mocker")

def test_max_outsize_exceeds_returndatasize(get_contract):
    sc = """
@external
def foo() -> Bytes[7]:
    return raw_call(0x0000000000000000000000000000000000000004, b"moose", max_outsize=7)
    """
    c = get_contract(sc)
    assert c.foo() == b"moose"

def test_raw_call_non_memory(get_contract):
    sc = """
_foo: Bytes[5]
@external
def foo() -> Bytes[5]:
    self._foo = b"moose"
    return raw_call(0x0000000000000000000000000000000000000004, self._foo, max_outsize=5)
    """
    c = get_contract(sc)
    assert c.foo() == b"moose"

def test_returndatasize_exceeds_max_outsize(get_contract):
    sc = """
@external
def foo() -> Bytes[3]:
    return raw_call(0x0000000000000000000000000000000000000004, b"moose", max_outsize=3)
    """
    c = get_contract(sc)
    assert c.foo() == b"moo"

def test_returndatasize_matches_max_outsize(get_contract):
    sc = """
@external
def foo() -> Bytes[5]:
    return raw_call(0x0000000000000000000000000000000000000004, b"moose", max_outsize=5)
    """
    c = get_contract(sc)
    assert c.foo() == b"moose"

def test_multiple_levels(w3, get_contract_with_gas_estimation):
    ic = """
@external
def returnten() -> int128:
    return 10
    """

    c = get_contract_with_gas_estimation(ic)

    oc = """
@external
def create_and_call_returnten(inp: address) -> int128:
    _x: address = create_minimal_proxy_to(inp)
    _o: int128 = extract32(raw_call(_x, b"\\xd0\\x1f\\xb1\\xb8", max_outsize=32, gas=50000), 0, output_type=int128)  # noqa: E501
    return _o

@external
def create_and_return_proxy(inp: address) -> address:
    _x: address = create_minimal_proxy_to(inp)
    return _x
    """

    c2 = get_contract_with_gas_estimation(oc)
    assert c2.create_and_call_returnten(c.address) == 10
    c2.create_and_call_returnten(c.address, transact={})

    _, pre, cc = eip1167_bytecode()

    c3 = c2.create_and_return_proxy(c.address, call={})
    c2.create_and_return_proxy(c.address, transact={})

    c3_contract_code = w3.to_bytes(w3.eth.get_code(c3))

    assert c3_contract_code[:10] == HexBytes(pre)
    assert c3_contract_code[-15:] == HexBytes(cc)

    print("Passed proxy test")
    # TODO: This one is special
    # print(f'Gas consumed: {(chain.head_state.receipts[-1].gas_used - chain.head_state.receipts[-2].gas_used - chain.last_tx.intrinsic_gas_used)}')  # noqa: E501


def test_multiple_levels2(assert_tx_failed, get_contract_with_gas_estimation):
    ic = """
@external
def returnten() -> int128:
    raise
    """

    c = get_contract_with_gas_estimation(ic)

    oc = """
@external
def create_and_call_returnten(inp: address) -> int128:
    _x: address = create_minimal_proxy_to(inp)
    _o: int128 = extract32(raw_call(_x, b"\\xd0\\x1f\\xb1\\xb8", max_outsize=32, gas=50000), 0, output_type=int128)  # noqa: E501
    return _o

@external
def create_and_return_proxy(inp: address) -> address:
    return create_minimal_proxy_to(inp)
    """

    c2 = get_contract_with_gas_estimation(oc)

    assert_tx_failed(lambda: c2.create_and_call_returnten(c.address))

    print("Passed minimal proxy exception test")

def test_delegate_call(w3, get_contract):
    ic = """
a: address  # this is required for storage alignment...
owners: public(address[5])

@external
def set_owner(i: int128, o: address):
    self.owners[i] = o
    """

    inner_contract = get_contract(ic)

    oc = """
owner_setter_contract: public(address)
owners: public(address[5])

@external
def __init__(_osc: address):
    self.owner_setter_contract = _osc

@external
def set(i: int128, owner: address):
    # delegate setting owners to other contract.s
    _c: Bytes[68] = concat(method_id("set_owner(int128,address)"), convert(i, bytes32), convert(owner, bytes32))  # noqa: E501
    raw_call(
        self.owner_setter_contract,
        _c,
        gas=msg.gas,
        max_outsize=0,
        is_delegate_call=True
    )
    """

    a0, a1, a2 = w3.eth.accounts[:3]
    outer_contract = get_contract(oc, *[inner_contract.address])

    inner_contract.set_owner(1, a2, transact={})
    assert inner_contract.owners(1) == a2

    assert outer_contract.owner_setter_contract() == inner_contract.address
    assert outer_contract.owners(1) is None

    tx_hash = outer_contract.set(1, a1, transact={})
    assert w3.eth.get_transaction_receipt(tx_hash)["status"] == 1
    assert outer_contract.owners(1) == a1


def test_gas(get_contract, assert_tx_failed):
    ic = """
bar: bytes32

@external
def foo(_bar: bytes32):
    self.bar = _bar
    """

    inner_contract = get_contract(ic)

    oc = """
@external
def foo_call(_a: address):
    _c: Bytes[40] = concat(
        method_id("foo(bytes32)"),
        0x0000000000000000000000000000000000000000000000000000000000000001
    )
    raw_call(_a, _c, max_outsize=0{})
    """

    outer_contract = get_contract(oc.format(""))
    outer_contract.foo_call(inner_contract.address)

    outer_contract = get_contract(oc.format(", gas=50000"))
    outer_contract.foo_call(inner_contract.address)

    outer_contract = get_contract(oc.format(", gas=15000"))
    assert_tx_failed(lambda: outer_contract.foo_call(inner_contract.address))


def test_static_call(get_contract):
    ts = """
@external
@view
def foo() -> int128:
    return 42
"""

    cs = """
@external
@view
def foo(_a: address) -> int128:
    _r: Bytes[32] = raw_call(
        _a,
        method_id("foo()"),
        max_outsize=32,
        is_static_call=True,
    )
    return convert(_r, int128)
    """

    target = get_contract(ts)
    caller = get_contract(cs)

    assert caller.foo(target.address) == 42

def test_forward_calldata(get_contract, w3, keccak):
    ts = """
@external
def foo() -> uint256:
    return 123
    """

    cs = """
target: address

@external
def set_target(_t: address):
    self.target = _t

@external
def __default__():
    assert 123 == _abi_decode(raw_call(self.target, msg.data, max_outsize=32), uint256)
    """

    target = get_contract(ts)

    caller = get_contract(cs)
    caller.set_target(target.address, transact={})

    sig = keccak("foo()".encode()).hex()[:10]
    w3.eth.send_transaction({"to": caller.address, "data": sig})

def test_static_call_fails_nonpayable(get_contract, assert_tx_failed):
    ts = """
baz: int128

@external
def foo() -> int128:
    self.baz = 31337
    return self.baz
"""

    cs = """
@external
@view
def foo(_a: address) -> int128:
    _r: Bytes[32] = raw_call(
        _a,
        method_id("foo()"),
        max_outsize=32,
        is_static_call=True,
    )
    return convert(_r, int128)
    """

    target = get_contract(ts)
    caller = get_contract(cs)

    assert_tx_failed(lambda: caller.foo(target.address))

def test_checkable_raw_call(get_contract, assert_tx_failed):
    ts = """
baz: int128
@external
def fail1(sr: bool):
    if sr:
        raise "fail"

@external
def fail2(sr: bool) -> int128:
    if sr:
        self.baz = self.baz + 1
    return self.baz
"""

    cs = """
@external
@view
def foo(_a: address, sr: bool) -> uint256:
    s: bool = True
    r: Bytes[32] = b""
    s, r = raw_call(
        _a,
        _abi_encode(sr, method_id=method_id("fail1(bool)")),
        max_outsize=32,
        is_static_call=True,
        revert_on_failure=False,
    )
    assert s == (not sr)
    return 1

@external
@view
def bar(_a: address, sr: bool) -> uint256:
    s: bool = True
    r: Bytes[32] = b""
    s, r = raw_call(
        _a,
        _abi_encode(sr, method_id=method_id("fail2(bool)")),
        max_outsize=32,
        is_static_call=True,
        revert_on_failure=False,
    )
    assert s == (not sr)
    return 2

@external
@nonpayable
def baz(_a: address, sr: bool) -> uint256:
    s: bool = True
    s = raw_call(
        _a,
        _abi_encode(sr, method_id=method_id("fail1(bool)")),
        revert_on_failure=False,
    )
    assert s == (not sr)
    return 3
    """

    target = get_contract(ts)
    caller = get_contract(cs)

    assert caller.foo(target.address, True) == 1
    assert caller.foo(target.address, False) == 1
    assert caller.bar(target.address, True) == 2
    assert caller.bar(target.address, False) == 2
    assert caller.baz(target.address, True) == 3
    assert caller.baz(target.address, False) == 3

uncompilable_code = [
    (
        """
@external
@view
def foo(_a: address):
    raw_call(_a, method_id("foo()"))
    """,
        StateAccessViolation,
    ),
    (
        """
@external
def foo(_a: address):
    raw_call(_a, method_id("foo()"), is_delegate_call=True, is_static_call=True)
    """,
        ArgumentException,
    ),
    (
        """
@external
@view
def foo(_a: address):
    raw_call(_a, 256)
    """,
        InvalidType,
    ),
]

@pytest.mark.parametrize("source_code,exc", uncompilable_code)
def test_invalid_type_exception(
    assert_compile_failed, get_contract_with_gas_estimation, source_code, exc
):
    assert_compile_failed(lambda: get_contract_with_gas_estimation(source_code), exc)