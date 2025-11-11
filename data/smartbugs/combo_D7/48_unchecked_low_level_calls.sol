pragma solidity ^0.4.25; 
contract demo{
    bytes4 private id;
    function transfer(address from, address caddress, address[] _tos, uint[] v) public returns (bool) {
        id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i=0; i<_tos.length; i++){
            caddress.call(id, from, _tos[i], v[i]);
        }
        return true;
    }
}