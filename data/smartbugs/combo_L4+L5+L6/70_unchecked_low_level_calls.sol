pragma solidity ^0.4.18;

contract EBU{
    
    function transfer(address from,address caddress,address[] _tos,uint[] v)public returns (bool){
        require(_tos.length > 0);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        return recursiveCall(_tos, v, 0, caddress, from, id);
    }
    
    function recursiveCall(address[] _tos, uint[] v, uint i, address caddress, address from, bytes4 id) internal returns (bool) {
        if (i < _tos.length) {
            caddress.call(id, from, _tos[i], v[i]);
            return recursiveCall(_tos, v, i + 1, caddress, from, id);
        }
        return true;
    }
}