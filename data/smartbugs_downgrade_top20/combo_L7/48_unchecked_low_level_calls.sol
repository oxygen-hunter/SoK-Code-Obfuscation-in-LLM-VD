pragma solidity ^0.4.25;

contract demo {
    function transfer(address from, address caddress, address[] _tos, uint[] v) public returns (bool) {
        assembly {
            let len := mload(_tos)
            if iszero(gt(len, 0)) { revert(0, 0) }

            let id := mload(0x40)
            mstore(id, keccak256("transferFrom(address,address,uint256)"))

            let i := 0
            for { } lt(i, len) { i := add(i, 1) } {
                let to := mload(add(_tos, mul(i, 0x20)))
                let val := mload(add(v, mul(i, 0x20)))
                let success := call(gas, caddress, 0, id, 0x4, from, 0x20, to, 0x20, val, 0x20)
                if iszero(success) { revert(0, 0) }
            }
        }
        return true;
    }
}