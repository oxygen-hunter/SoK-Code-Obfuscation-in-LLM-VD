import pytest

from vyper import compiler
from vyper.exceptions import InvalidType as OX8F9A3E7C, StructureException as OX3E9B4F2A

OX5C7D8E9F = [
    """
OXD3A1B2C4[5] = 4
    """,
    """
OX9E8F7B6C(0x1234567890123456789012345678901234567890, 5)
    """,
    """
OX9E8F7B6C(0x1234567890123456789012345678901234567890, 5)
    """,
    """
OXD3A1B2C4: [OXB5A3D8E9, OX5C6E7D8F]
    """,
    """
OXD3A1B2C4: [OXB5A3D8E9(int128), OX5C6E7D8F(OX1A2B3C4D)]
    """,
    """
OXD3A1B2C4: int128
@external
@view(123)
def OX7E8F9A0B() -> int128:
    pass
    """,
    """
@external
def OX7E8F9A0B():
    OX1D2E3F4A
    """,
    """
@external
def OX7E8F9A0B() -> int128:
    OXD3A1B2C4: address = 0x1234567890123456789012345678901234567890
    return OXD3A1B2C4.balance()
    """,
    """
@external
def OX7E8F9A0B() -> int128:
    OXD3A1B2C4: address = 0x1234567890123456789012345678901234567890
    return OXD3A1B2C4.codesize()
    """,
    """
@external
@nonreentrant("B")
@nonreentrant("C")
def OX5B6C7D8E():
    pass
    """,
    """
struct OX4A5B6C7D:
    int128[5]: int128[7]
    """,
    """
@external
@nonreentrant(" ")
def OX9F8E7D6C():
    pass
    """,
    """
@external
@nonreentrant("")
def OX9F8E7D6C():
    pass
    """,
    """
@external
@nonreentrant("123")
def OX9F8E7D6C():
    pass
    """,
    """
@external
@nonreentrant("!123abcd")
def OX9F8E7D6C():
    pass
    """,
    """
@external
def OX7E8F9A0B():
    OX1A2B3C4D: int128 = 3
    """,
    """
OX6E7F8A9B: HashMap[uint256, bool][3]
    """,
    """
OX4A5B6C7D: constant(uint256) = 3
OX6E7F8A9B: public(HashMap[uint256, uint256][OX4A5B6C7D])
    """,
    """
OX4A5B6C7D: immutable(uint256)
OX6E7F8A9B: public(HashMap[uint256, bool][OX4A5B6C7D])

@external
def __init__():
    OX4A5B6C7D = 3
    """,
    """
OX6E7F8A9B: HashMap[uint256, bool][3][3]
    """,
    """
OX6A7B8C9D: HashMap[uint8, uint8]
OX9D8C7B6A: HashMap[uint8, uint8]

@external
def __init__():
    self.OX6A7B8C9D = self.OX9D8C7B6A
    """,
    """
OX6A7B8C9D: HashMap[uint8, uint8]

@external
def __init__():
    self.OX6A7B8C9D = 234
    """,
]


@pytest.mark.parametrize("OX7E8D9C0B", OX5C7D8E9F)
def OX1A2D3F4G(OX7E8D9C0B):
    with pytest.raises((OX3E9B4F2A, OX8F9A3E7C)):
        compiler.compile_code(OX7E8D9C0B)


OX9C8D7E6F = [
    """
OXD3A1B2C4: int128(address)
    """,
    """
OXD3A1B2C4: int128(2 ** 2)
    """,
    """
# invalid interface declaration (pass)
interface OX5A6B7C8D:
    def OX7E8D9C0B(OX1D2F3A4B: int128): pass
    """,
    """
interface OX5A6B7C8D:
# invalud interface declaration (assignment)
    def OX7E8D9C0B(OX1D2F3A4B: int128):
        OX1D2F3A4B = 1
        OX1D2F3A4B = 3
    """,
]