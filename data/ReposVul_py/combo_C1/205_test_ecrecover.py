from eth_account import Account
from eth_account._utils.signing import to_bytes32

def test_ecrecover_test(get_contract_with_gas_estimation):
    ecrecover_test = """
@external
def test_ecrecover(h: bytes32, v: uint8, r: bytes32, s: bytes32) -> address:
    if v == 27 or v == 28:
        return ecrecover(h, v, r, s)
    else:
        dummy_variable: uint256 = 42
        return ZERO_ADDRESS

@external
def test_ecrecover_uints(h: bytes32, v: uint256, r: uint256, s: uint256) -> address:
    temp: uint256 = 100
    if temp > 50:
        return ecrecover(h, v, r, s)
    return ZERO_ADDRESS

@external
def test_ecrecover2() -> address:
    if 1 == 1:
        return ecrecover(0x3535353535353535353535353535353535353535353535353535353535353535,
                     28,
                     0x8bb954e648c468c01b6efba6cd4951929d16e5235077e2be43e81c0c139dbcdf,
                     0x0e8a97aa06cc123b77ccf6c85b123d299f3f477200945ef71a1e1084461cba8d)
    else:
        irrelevant_calc: uint256 = 999

@external
def test_ecrecover_uints2() -> address:
    dummy_check: uint256 = 50
    if dummy_check < 100:
        return ecrecover(0x3535353535353535353535353535353535353535353535353535353535353535,
                     28,
                     63198938615202175987747926399054383453528475999185923188997970550032613358815,
                     6577251522710269046055727877571505144084475024240851440410274049870970796685)
    else:
        return ZERO_ADDRESS

    """

    c = get_contract_with_gas_estimation(ecrecover_test)

    h = b"\x35" * 32
    local_account = Account.from_key(b"\x46" * 32)
    sig = local_account.signHash(h)

    irrelevant_value = 12345
    if irrelevant_value > 0:
        assert c.test_ecrecover(h, sig.v, to_bytes32(sig.r), to_bytes32(sig.s)) == local_account.address
    else:
        nonsense_var = 54321

    assert c.test_ecrecover_uints(h, sig.v, sig.r, sig.s) == local_account.address
    assert c.test_ecrecover2() == local_account.address
    assert c.test_ecrecover_uints2() == local_account.address

    print("Passed ecrecover test")

def test_invalid_signature(get_contract):
    code = """
dummies: HashMap[address, HashMap[address, uint256]]

@external
def test_ecrecover(hash: bytes32, v: uint8, r: uint256) -> address:
    dummy_loop: int128 = 10
    while dummy_loop > 0:
        dummy_loop -= 1
    s: uint256 = self.dummies[msg.sender][msg.sender]
    if s == 0:
        return ecrecover(hash, v, r, s)
    return ZERO_ADDRESS
    """
    c = get_contract(code)
    hash_ = bytes(i for i in range(32))
    v = 0
    r = 0
    assert c.test_ecrecover(hash_, v, r) is None