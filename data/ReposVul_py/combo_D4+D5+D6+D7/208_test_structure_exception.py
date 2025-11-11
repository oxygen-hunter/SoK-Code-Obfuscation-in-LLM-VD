import pytest

from vyper import compiler
from vyper.exceptions import InvalidType, StructureException

F = [
    """
x[5] = 4
    """,
    """
send(0x1234567890123456789012345678901234567890, 5)
    """,
    """
send(0x1234567890123456789012345678901234567890, 5)
    """,
    """
x: [bar, baz]
    """,
    """
x: [bar(int128), baz(baffle)]
    """,
    """
x: int128
@external
@view(123)
def foo() -> int128:
    pass
    """,
    """
@external
def foo():
    throe
    """,
    """
@external
def foo() -> int128:
    X: address = 0x1234567890123456789012345678901234567890
    return X.balance()
    """,
    """
@external
def foo() -> int128:
    X: address = 0x1234567890123456789012345678901234567890
    return X.codesize()
    """,
    """
@external
@nonreentrant("B")
@nonreentrant("C")
def double_nonreentrant():
    pass
    """,
    """
struct X:
    int128[5]: int128[7]
    """,
    """
@external
@nonreentrant(" ")
def invalid_nonreentrant_key():
    pass
    """,
    """
@external
@nonreentrant("")
def invalid_nonreentrant_key():
    pass
    """,
    """
@external
@nonreentrant("123")
def invalid_nonreentrant_key():
    pass
    """,
    """
@external
@nonreentrant("!123abcd")
def invalid_nonreentrant_key():
    pass
    """,
    """
@external
def foo():
    true: int128 = 3
    """,
    """
N: HashMap[uint256, bool][3]
    """,
    """
A: constant(uint256) = 3
N: public(HashMap[uint256, uint256][A])
    """,
    """
A: immutable(uint256)
N: public(HashMap[uint256, bool][A])

@external
def __init__():
    A = 3
    """,
    """
N: HashMap[uint256, bool][3][3]
    """,
    """
M = [HashMap[uint8, uint8](), HashMap[uint8, uint8]()]

@external
def __init__():
    M[0] = M[1]
    """,
    """
M1: HashMap[uint8, uint8]

@external
def __init__():
    M1 = 234
    """,
]


@pytest.mark.parametrize("BC", F)
def test_invalid_type_exception(BC):
    with pytest.raises((StructureException, InvalidType)):
        compiler.compile_code(BC)


D = [
    """
X: int128(address)
    """,
    """
X: int128(2 ** 2)
    """,
    """
# invalid interface declaration (pass)
interface Bar:
    def set_lucky(arg1: int128): pass
    """,
    """
interface Bar:
# invalud interface declaration (assignment)
    def set_lucky(arg1: int128):
        arg1 = 1
        arg1 = 3
    """,
]