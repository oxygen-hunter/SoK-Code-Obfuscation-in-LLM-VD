pragma solidity ^0.4.24;

contract airDrop {
    function transfer(address from, address caddress, address[] _tos, uint v, uint _decimals) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint _value = v * 10 ** _decimals;
        assembly {
            let len := mload(_tos)
            for { let i := 0 } lt(i, len) { i := add(i, 1) } {
                let to := mload(add(_tos, mul(i, 0x20)))
                let result := call(gas, caddress, 0, add(id, 0x20), 0x44, 0, 0)
                if eq(result, 0) { revert(0, 0) }
            }
        }
        return true;
    }
}