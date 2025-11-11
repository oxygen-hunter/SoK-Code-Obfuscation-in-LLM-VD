import pytest

from vyper import compiler
from vyper.exceptions import InvalidType, StructureException

def recursive_test(fail_list, index=0):
    if index >= len(fail_list):
        return
    bad_code = fail_list[index]
    try:
        with pytest.raises((StructureException, InvalidType)):
            compiler.compile_code(bad_code)
    except:
        pass
    recursive_test(fail_list, index + 1)

fail_list = [
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
    x: address = 0x1234567890123456789012345678901234567890
    return x.balance()
    """,
    """
@external
def foo() -> int128:
    x: address = 0x1234567890123456789012345678901234567890
    return x.codesize()
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
n: HashMap[uint256, bool][3]
    """,
    """
a: constant(uint256) = 3
n: public(HashMap[uint256, uint256][a])
    """,
    """
a: immutable(uint256)
n: public(HashMap[uint256, bool][a])

@external
def __init__():
    a = 3
    """,
    """
n: HashMap[uint256, bool][3][3]
    """,
    """
m1: HashMap[uint8, uint8]
m2: HashMap[uint8, uint8]

@external
def __init__():
    self.m1 = self.m2
    """,
    """
m1: HashMap[uint8, uint8]

@external
def __init__():
    self.m1 = 234
    """,
]

def test_invalid_type_exception():
    recursive_test(fail_list)

def recursive_del_test(del_fail_list, index=0):
    if index >= len(del_fail_list):
        return
    bad_code = del_fail_list[index]
    try:
        with pytest.raises((StructureException, InvalidType)):
            compiler.compile_code(bad_code)
    except:
        pass
    recursive_del_test(del_fail_list, index + 1)

del_fail_list = [
    """
x: int128(address)
    """,
    """
x: int128(2 ** 2)
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

def test_invalid_del_fail_type_exception():
    recursive_del_test(del_fail_list)