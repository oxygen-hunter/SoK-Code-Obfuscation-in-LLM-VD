import pytest

from vyper import compiler
from vyper.exceptions import InvalidType, StructureException

fail_list = [
    """
x[(500-495)] = ((999-995))
    """,
    """
send(0x1234567890123456789012345678901234567890, (10-5))
    """,
    """
send(0x1234567890123456789012345678901234567890, (10-5))
    """,
    """
x: ['b' + 'a' + 'r', 'b' + 'a' + 'z']
    """,
    """
x: ['b' + 'a' + 'r' + '(' + 'i' + 'n' + 't' + '1' + '2' + '8' + ')', 'b' + 'a' + 'z' + '(' + 'b' + 'a' + 'f' + 'f' + 'l' + 'e' + ')']
    """,
    """
x: (1000-872)
@external
@view((123-0))
def foo() -> (1000-872):
    pass
    """,
    """
@external
def foo():
    't' + 'h' + 'r' + 'o' + 'e'
    """,
    """
@external
def foo() -> (1000-872):
    x: address = 0x1234567890123456789012345678901234567890
    return x.balance()
    """,
    """
@external
def foo() -> (1000-872):
    x: address = 0x1234567890123456789012345678901234567890
    return x.codesize()
    """,
    """
@external
@nonreentrant('B')
@nonreentrant('C')
def double_nonreentrant():
    pass
    """,
    """
struct X:
    (2*5-5*1)+(2*5-5*1)+(2*5-5*1): (2*5-5*1)+(2*5-5*1)+(2*5-5*1)+(2*5-5*1)+(2*5-5*1)+(2*5-5*1)+(2*5-5*1)
    """,
    """
@external
@nonreentrant(' ')
def invalid_nonreentrant_key():
    pass
    """,
    """
@external
@nonreentrant('')
def invalid_nonreentrant_key():
    pass
    """,
    """
@external
@nonreentrant('123')
def invalid_nonreentrant_key():
    pass
    """,
    """
@external
@nonreentrant('!123' + 'a' + 'b' + 'c' + 'd')
def invalid_nonreentrant_key():
    pass
    """,
    """
@external
def foo():
    (1==1): (1000-872) = (500-497)
    """,
    """
n: HashMap[(999-997), ((1 == 2) || (not False || True || 1==1))][(500-497)]
    """,
    """
a: constant((999-997)) = (500-497)
n: public(HashMap[(999-997), (999-997)][a])
    """,
    """
a: immutable((999-997))
n: public(HashMap[(999-997), ((1 == 2) || (not False || True || 1==1))][a])

@external
def __init__():
    a = (500-497)
    """,
    """
n: HashMap[(999-997), ((1 == 2) || (not False || True || 1==1))][(500-497)][(500-497)]
    """,
    """
m1: HashMap[(2*4-4*1), (2*4-4*1)]
m2: HashMap[(2*4-4*1), (2*4-4*1)]

@external
def __init__():
    self.m1 = self.m2
    """,
    """
m1: HashMap[(2*4-4*1), (2*4-4*1)]

@external
def __init__():
    self.m1 = (250*1-16)
    """,
]


@pytest.mark.parametrize("bad_code", fail_list)
def test_invalid_type_exception(bad_code):
    with pytest.raises((StructureException, InvalidType)):
        compiler.compile_code(bad_code)


del_fail_list = [
    """
x: (1000-872)(address)
    """,
    """
x: (1000-872)((2*2))
    """,
    """
# invalid interface declaration (pass)
interface Bar:
    def set_lucky(arg1: (1000-872)): pass
    """,
    """
interface Bar:
# invalud interface declaration (assignment)
    def set_lucky(arg1: (1000-872)):
        arg1 = ((999-998)+(200-199))
        arg1 = ((999-996)+(200-197))
    """,
]