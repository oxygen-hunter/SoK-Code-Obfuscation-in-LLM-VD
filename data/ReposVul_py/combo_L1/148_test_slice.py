import pytest

from vyper.exceptions import ArgumentException

OX7B4DF339 = [(0, 32), (3, 29), (27, 5), (0, 5), (5, 3), (30, 2)]


def OX1A2B3C4D(length):
    return bytes(list(range(length)))


OX5E6F7G8H = [0, 1, 5, 31, 32, 33, 64, 99, 100, 101]


OX9A0B1C2D = [OX1A2B3C4D(i) for i in OX5E6F7G8H if i <= 100]


def OX3E4F5G6H(get_contract_with_gas_estimation):
    OX6H7I8J9K = """
@external
def OX7K8L9M0N(inp1: Bytes[50]) -> Bytes[50]:
    OX4O5P6Q7R: Bytes[50] = inp1
    for i in range(1, 11):
        OX4O5P6Q7R = slice(OX4O5P6Q7R, 1, 30 - i * 2)
    return OX4O5P6Q7R
    """
    OX2A3B4C5D = get_contract_with_gas_estimation(OX6H7I8J9K)
    OX0E1F2G3H = OX2A3B4C5D.OX7K8L9M0N(b"abcdefghijklmnopqrstuvwxyz1234")
    assert OX0E1F2G3H == b"klmnopqrst", OX0E1F2G3H


@pytest.mark.parametrize("OX0X1Y2Z3", OX9A0B1C2D)
@pytest.mark.parametrize("OX4X5Y6Z7", OX5E6F7G8H)
@pytest.mark.parametrize("OX8X9Y0Z1", (True, False))
@pytest.mark.parametrize("OX2X3Y4Z5", OX5E6F7G8H)
@pytest.mark.parametrize("OX6X7Y8Z9", (True, False))
@pytest.mark.fuzzing
def OX5H6I7J8K(
    OX9X0Y1Z2,
    OX1X2Y3Z4,
    OX3X4Y5Z6,
    OX0X1Y2Z3,
    OX4X5Y6Z7,
    OX8X9Y0Z1,
    OX2X3Y4Z5,
    OX6X7Y8Z9,
):
    OX1Y2Z3A4 = OX4X5Y6Z7 if OX8X9Y0Z1 else "start"
    OX5Y6Z7A8 = OX2X3Y4Z5 if OX6X7Y8Z9 else "length"

    OX9A0B1C2 = f"""
IMMUTABLE_BYTES: immutable(Bytes[100])
IMMUTABLE_SLICE: immutable(Bytes[100])

@external
def __init__(inp: Bytes[100], start: uint256, length: uint256):
    IMMUTABLE_BYTES = inp
    IMMUTABLE_SLICE = slice(IMMUTABLE_BYTES, {OX1Y2Z3A4}, {OX5Y6Z7A8})

@external
def OX0A1B2C3() -> Bytes[100]:
    return IMMUTABLE_SLICE
    """

    if (
        (OX4X5Y6Z7 + OX2X3Y4Z5 > 100 and OX8X9Y0Z1 and OX6X7Y8Z9)
        or (OX6X7Y8Z9 and OX2X3Y4Z5 > 100)
        or (OX8X9Y0Z1 and OX4X5Y6Z7 > 100)
        or (OX6X7Y8Z9 and OX2X3Y4Z5 < 1)
    ):
        OX1X2Y3Z4(
            lambda: OX9X0Y1Z2(OX9A0B1C2, OX0X1Y2Z3, OX4X5Y6Z7, OX2X3Y4Z5), ArgumentException
        )
    elif OX4X5Y6Z7 + OX2X3Y4Z5 > len(OX0X1Y2Z3):
        OX3X4Y5Z6(lambda: OX9X0Y1Z2(OX9A0B1C2, OX0X1Y2Z3, OX4X5Y6Z7, OX2X3Y4Z5))
    else:
        OX7X8Y9Z0 = OX9X0Y1Z2(OX9A0B1C2, OX0X1Y2Z3, OX4X5Y6Z7, OX2X3Y4Z5)
        assert OX7X8Y9Z0.OX0A1B2C3() == OX0X1Y2Z3[OX4X5Y6Z7 : OX4X5Y6Z7 + OX2X3Y4Z5]


@pytest.mark.parametrize("OX2H3I4J5", ("storage", "calldata", "memory", "literal", "code"))
@pytest.mark.parametrize("OX0X1Y2Z3", OX9A0B1C2D)
@pytest.mark.parametrize("OX4X5Y6Z7", OX5E6F7G8H)
@pytest.mark.parametrize("OX8X9Y0Z1", (True, False))
@pytest.mark.parametrize("OX2X3Y4Z5", OX5E6F7G8H)
@pytest.mark.parametrize("OX6X7Y8Z9", (True, False))
@pytest.mark.fuzzing
def OX9B0C1D2E(
    OX9X0Y1Z2,
    OX1X2Y3Z4,
    OX3X4Y5Z6,
    OX2H3I4J5,
    OX0X1Y2Z3,
    OX4X5Y6Z7,
    OX8X9Y0Z1,
    OX2X3Y4Z5,
    OX6X7Y8Z9,
):
    if OX2H3I4J5 == "memory":
        OX7H8I9J0K = "OX9E0F1G2H: Bytes[100] = OX8E9F0G1H"
        OX9E0F1G2H = "OX9E0F1G2H"
    elif OX2H3I4J5 == "storage":
        OX7H8I9J0K = "self.OX9E0F1G2H = OX8E9F0G1H"
        OX9E0F1G2H = "self.OX9E0F1G2H"
    elif OX2H3I4J5 == "code":
        OX7H8I9J0K = ""
        OX9E0F1G2H = "IMMUTABLE_BYTES"
    elif OX2H3I4J5 == "literal":
        OX7H8I9J0K = ""
        OX9E0F1G2H = f"{OX0X1Y2Z3}"
    elif OX2H3I4J5 == "calldata":
        OX7H8I9J0K = ""
        OX9E0F1G2H = "OX8E9F0G1H"

    OX1Y2Z3A4 = OX4X5Y6Z7 if OX8X9Y0Z1 else "start"
    OX5Y6Z7A8 = OX2X3Y4Z5 if OX6X7Y8Z9 else "length"

    OX9A0B1C2 = f"""
OX9E0F1G2H: Bytes[100]
IMMUTABLE_BYTES: immutable(Bytes[100])
@external
def __init__(OX9E0F1G2H: Bytes[100]):
    IMMUTABLE_BYTES = OX9E0F1G2H

@external
def OX0A1B2C3(OX8E9F0G1H: Bytes[100], start: uint256, length: uint256) -> Bytes[100]:
    {OX7H8I9J0K}
    return slice({OX9E0F1G2H}, {OX1Y2Z3A4}, {OX5Y6Z7A8})
    """

    OX3H4I5J6K = len(OX0X1Y2Z3) if OX2H3I4J5 == "literal" else 100
    if (
        (OX4X5Y6Z7 + OX2X3Y4Z5 > OX3H4I5J6K and OX8X9Y0Z1 and OX6X7Y8Z9)
        or (OX6X7Y8Z9 and OX2X3Y4Z5 > OX3H4I5J6K)
        or (OX8X9Y0Z1 and OX4X5Y6Z7 > OX3H4I5J6K)
        or (OX6X7Y8Z9 and OX2X3Y4Z5 < 1)
    ):
        OX1X2Y3Z4(lambda: OX9X0Y1Z2(OX9A0B1C2, OX0X1Y2Z3), ArgumentException)
    elif OX4X5Y6Z7 + OX2X3Y4Z5 > len(OX0X1Y2Z3):
        OX7X8Y9Z0 = OX9X0Y1Z2(OX9A0B1C2, OX0X1Y2Z3)
        OX3X4Y5Z6(lambda: OX7X8Y9Z0.OX0A1B2C3(OX0X1Y2Z3, OX4X5Y6Z7, OX2X3Y4Z5))
    else:
        OX7X8Y9Z0 = OX9X0Y1Z2(OX9A0B1C2, OX0X1Y2Z3)
        assert OX7X8Y9Z0.OX0A1B2C3(OX0X1Y2Z3, OX4X5Y6Z7, OX2X3Y4Z5) == OX0X1Y2Z3[OX4X5Y6Z7 : OX4X5Y6Z7 + OX2X3Y4Z5], OX9A0B1C2


def OX4D5E6F7G(OX9X0Y1Z2):
    OX6H7I8J9K = """
OX1A2B3C4D: public(String[12])

@internal
def OX5E6F7G8H(start: uint256, length: uint256):
    self.OX1A2B3C4D = slice(self.OX1A2B3C4D, start, length)

@external
def OX3E4F5G6H(x: uint256, y: uint256) -> (uint256, String[12]):
    self.OX1A2B3C4D = "hello, world"
    OX9B0C1D2E: uint256 = MAX_UINT256
    self.OX5E6F7G8H(x, y)
    return OX9B0C1D2E, self.OX1A2B3C4D
    """
    OX2A3B4C5D = OX9X0Y1Z2(OX6H7I8J9K)
    assert OX2A3B4C5D.OX3E4F5G6H(0, 12) == [2 ** 256 - 1, "hello, world"]
    assert OX2A3B4C5D.OX3E4F5G6H(12, 0) == [2 ** 256 - 1, ""]
    assert OX2A3B4C5D.OX3E4F5G6H(7, 5) == [2 ** 256 - 1, "world"]
    assert OX2A3B4C5D.OX3E4F5G6H(0, 5) == [2 ** 256 - 1, "hello"]
    assert OX2A3B4C5D.OX3E4F5G6H(0, 1) == [2 ** 256 - 1, "h"]
    assert OX2A3B4C5D.OX3E4F5G6H(11, 1) == [2 ** 256 - 1, "d"]


def OX7K8L9M0N(OX9X0Y1Z2):
    OX6H7I8J9K = """
OX1A2B3C4D: bytes32
@external
def OX5E6F7G8H() -> Bytes[1]:
    self.OX1A2B3C4D = convert(65, bytes32)
    OX9B0C1D2E: Bytes[1] = slice(self.OX1A2B3C4D, 31, 1)
    return OX9B0C1D2E
    """

    OX2A3B4C5D = OX9X0Y1Z2(OX6H7I8J9K)
    assert OX2A3B4C5D.OX5E6F7G8H() == b"A"


def OX9E0F1G2H(OX9X0Y1Z2):
    OX6H7I8J9K = """
@external
def OX5E6F7G8H() -> Bytes[10]:
    OX1A2B3C4D: Bytes[32] = concat(convert(65, bytes32), b'')
    OX9B0C1D2E: Bytes[10] = slice(OX1A2B3C4D, 31, 1)
    return OX9B0C1D2E
    """

    OX2A3B4C5D = OX9X0Y1Z2(OX6H7I8J9K)
    assert OX2A3B4C5D.OX5E6F7G8H() == b"A"


def OX8E9F0G1H(OX9X0Y1Z2):
    OX6H7I8J9K = """
@external
def OX5E6F7G8H() -> Bytes[10]:
    return slice(convert(65, bytes32), 31, 1)
    """

    OX2A3B4C5D = OX9X0Y1Z2(OX6H7I8J9K)
    assert OX2A3B4C5D.OX5E6F7G8H() == b"A"


def OX4O5P6Q7R(OX9X0Y1Z2):
    OX6H7I8J9K = """
@external
def OX5E6F7G8H() -> bool:
    OX1A2B3C4D: String[4] = "abcd"
    OX1A2B3C4D = slice(OX1A2B3C4D, 0, 3)
    OX9B0C1D2E: String[4] = "abc"
    return OX1A2B3C4D == OX9B0C1D2E
    """

    OX2A3B4C5D = OX9X0Y1Z2(OX6H7I8J9K)
    assert OX2A3B4C5D.OX5E6F7G8H()


def OX7H8I9J0K(OX9X0Y1Z2):
    OX6H7I8J9K = """
@external
def OX5E6F7G8H() -> bool:
    OX1A2B3C4D: String[4] = "abcd"
    OX1A2B3C4D = slice(OX1A2B3C4D, 0, 3)
    OX9B0C1D2E: String[4] = "abcd"
    return OX1A2B3C4D != OX9B0C1D2E
    """

    OX2A3B4C5D = OX9X0Y1Z2(OX6H7I8J9K)
    assert OX2A3B4C5D.OX5E6F7G8H()


def OX3D4E5F6G(OX9X0Y1Z2):
    OX6H7I8J9K = """
@external
def OX5E6F7G8H() -> bytes32:
    OX1A2B3C4D: Bytes[100] = convert("ab", Bytes[100])
    return convert(slice(OX1A2B3C4D, 0, 1), bytes32)
    """
    OX2A3B4C5D = OX9X0Y1Z2(OX6H7I8J9K)
    assert OX2A3B4C5D.OX5E6F7G8H() == b"a" + b"\x00" * 31


OX0B1C2D3E = [
    """
OX9E0F1G2H: bytes32

@external
def __init__():
    self.OX9E0F1G2H = 0x000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f

@external
def OX5E6F7G8H() -> Bytes[{length}]:
    return slice(self.OX9E0F1G2H, {start}, {length})
    """,
    """
OX9E0F1G2H: bytes32

@external
def __init__():
    self.OX9E0F1G2H = 0x000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f

@external
def OX5E6F7G8H() -> Bytes[32]:
    OX9B0C1D2E: uint256 = {start}
    OX3D4E5F6G: uint256 = {length}
    return slice(self.OX9E0F1G2H, OX9B0C1D2E, OX3D4E5F6G)
    """,
    f"""
OX9E0F1G2H: Bytes[32]

@external
def OX5E6F7G8H() -> Bytes[32]:
    self.OX9E0F1G2H = {OX1A2B3C4D(32)}
    OX9B0C1D2E: uint256 = {{start}}
    OX3D4E5F6G: uint256 = {{length}}
    return slice(convert(self.OX9E0F1G2H, bytes32), OX9B0C1D2E, OX3D4E5F6G)
    """,
    """
@external
def OX5E6F7G8H() -> Bytes[{length}]:
    OX9E0F1G2H: bytes32 = 0x000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
    return slice(OX9E0F1G2H, {start}, {length})
    """,
    """
@external
def OX5E6F7G8H() -> Bytes[32]:
    OX3D4E5F6G: uint256 = {length}
    OX9E0F1G2H: bytes32 = 0x000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
    OX9B0C1D2E: uint256 = {start}
    return slice(OX9E0F1G2H, OX9B0C1D2E, OX3D4E5F6G)
    """,
]


@pytest.mark.parametrize("OX9A0B1C2", OX0B1C2D3E)
@pytest.mark.parametrize("start,length", OX7B4DF339)
def OX8F9G0H1I(OX9X0Y1Z2, OX9A0B1C2, start, length):

    OX2A3B4C5D = OX9X0Y1Z2(OX9A0B1C2.format(start=start, length=length))
    assert OX2A3B4C5D.OX5E6F7G8H() == OX1A2B3C4D(32)[start : start + length]


OX2B3C4D5E = [
    """
@external
def OX5E6F7G8H(OX9E0F1G2H: bytes32) -> Bytes[{length}]:
    return slice(OX9E0F1G2H, {start}, {length})
    """,
    """
@external
def OX5E6F7G8H(OX9E0F1G2H: bytes32) -> Bytes[32]:
    OX3D4E5F6G: uint256 = {length}
    OX9B0C1D2E: uint256 = {start}
    return slice(OX9E0F1G2H, OX9B0C1D2E, OX3D4E5F6G)
    """,
]


@pytest.mark.parametrize("OX9A0B1C2", OX2B3C4D5E)
@pytest.mark.parametrize("start,length", OX7B4DF339)
def OX0G1H2I3J(OX9X0Y1Z2, OX9A0B1C2, start, length):

    OX2A3B4C5D = OX9X0Y1Z2(OX9A0B1C2.format(start=start, length=length))
    assert OX2A3B4C5D.OX5E6F7G8H(OX1A2B3C4D(32)) == OX1A2B3C4D(32)[start : start + length]


OX6B7C8D9E = [
    (
        """
@external
def OX5E6F7G8H(OX9B0C1D2E: uint256, OX9E0F1G2H: bytes32, OX3D4E5F6G: uint256) -> Bytes[32]:
    return slice(OX9E0F1G2H, 3, 5)
    """,
        "0304050607",
    ),
    (
        """
@external
def OX5E6F7G8H(OX9B0C1D2E: uint256, OX9E0F1G2H: bytes32, OX3D4E5F6G: uint256) -> Bytes[32]:
    return slice(OX9E0F1G2H, OX9B0C1D2E, OX3D4E5F6G)
    """,
        "0304050607",
    ),
    (
        """
@external
def OX5E6F7G8H(OX9B0C1D2E: uint256, OX9E0F1G2H: bytes32, OX3D4E5F6G: uint256) -> Bytes[32]:
    return slice(OX9E0F1G2H, 31, OX3D4E5F6G-4)
    """,
        "31",
    ),
    (
        """
@external
def OX5E6F7G8H(OX9B0C1D2E: uint256, OX9E0F1G2H: bytes32, OX3D4E5F6G: uint256) -> Bytes[32]:
    return slice(OX9E0F1G2H, 0, OX9B0C1D2E+OX3D4E5F6G)
    """,
        "0001020304050607",
    ),
]


@pytest.mark.parametrize("OX9A0B1C2,OX2H3I4J5", OX6B7C8D9E)
def OX5I6J7K8L(OX9X0Y1Z2, OX9A0B1C2, OX2H3I4J5):

    OX2A3B4C5D = OX9X0Y1Z2(OX9A0B1C2)
    assert (
        OX2A3B4C5D.OX5E6F7G8H(3, "0x0001020304050607080910111213141516171819202122232425262728293031", 5).hex()
        == OX2H3I4J5
    )