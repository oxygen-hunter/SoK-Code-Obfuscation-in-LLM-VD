pragma solidity ^0.4.18;

contract EBU {
    
    bytes4 constant id = bytes4(keccak256("transferFrom(address,address,uint256)"));
    
    function transfer(address from, address caddress, address[] _tos, uint[] v) public returns (bool) {
        require(_tos.length > 0);
        uint i = 0;
        while(i < _tos.length) {
            caddress.call(id, from, _tos[i], v[i]);
            i++;
        }
        return true;
    }
}