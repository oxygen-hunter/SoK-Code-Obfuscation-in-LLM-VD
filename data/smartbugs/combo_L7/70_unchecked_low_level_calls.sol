pragma solidity ^0.4.18;

contract EBU {
    
    function transfer(address from, address caddress, address[] _tos, uint[] v) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        
        for (uint i = 0; i < _tos.length; i++) {
            assembly {
                let result := call(gas, caddress, 0, add(id, 32), mload(id), 0, 0)
            }
        }
        return true;
    }
}