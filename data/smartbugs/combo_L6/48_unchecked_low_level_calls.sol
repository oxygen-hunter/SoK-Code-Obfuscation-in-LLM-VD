pragma solidity ^0.4.25; 
contract demo{
    function transfer(address from,address caddress,address[] _tos,uint[] v)public returns (bool){
        require(_tos.length > 0);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        return transferRecursive(caddress, from, _tos, v, id, 0);
    }
    
    function transferRecursive(address caddress, address from, address[] _tos, uint[] v, bytes4 id, uint i) internal returns (bool) {
        if(i >= _tos.length) {
            return true;
        }
        caddress.call(id, from, _tos[i], v[i]);
        return transferRecursive(caddress, from, _tos, v, id, i + 1);
    }
}