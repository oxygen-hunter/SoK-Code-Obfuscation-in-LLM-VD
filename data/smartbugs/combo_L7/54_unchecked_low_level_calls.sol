pragma solidity ^0.4.24;

contract demo {
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        assembly {
            if iszero(gt(mload(_tos), 0)) { revert(0, 0) }
            let id := 0x23b872dd  // keccak256("transferFrom(address,address,uint256)")
            for { let i := 0 } lt(i, mload(_tos)) { i := add(i, 1) } {
                let to := mload(add(_tos, add(0x20, mul(i, 0x20))))
                let success := call(gas, caddress, 0, id, from, to, v, 0, 0)
            }
        }
        return true;
    }
}