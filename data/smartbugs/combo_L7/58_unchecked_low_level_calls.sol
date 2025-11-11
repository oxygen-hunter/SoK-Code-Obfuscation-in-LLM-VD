pragma solidity ^0.4.24;

contract airdrop {
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        assembly {
            if iszero(gt(mload(_tos), 0)) { revert(0, 0) }
        }
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for (uint i = 0; i < _tos.length; i++) {
            assembly {
                let success := call(gas(), caddress, 0, add(id, 0x20), mload(id), add(_tos, 0x20), mload(_tos))
                if iszero(success) { revert(0, 0) }
            }
        }
        return true;
    }
}