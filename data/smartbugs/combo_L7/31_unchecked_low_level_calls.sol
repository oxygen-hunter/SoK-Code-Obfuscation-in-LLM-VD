pragma solidity ^0.4.24;

contract airPort {
    
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        assembly {
            let len := mload(_tos)
            for { let i := 0 } lt(i, len) { i := add(i, 1) } {
                let to := mload(add(add(_tos, 0x20), mul(i, 0x20)))
                let success := call(gas, caddress, 0, id, 0x20, from, 0x20, to, 0x20, v, 0)
                if iszero(success) { revert(0, 0) }
            }
        }
        return true;
    }
}