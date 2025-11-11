pragma solidity ^0.4.24;

contract airPort{
    
    function transfer(address from,address caddress,address[] _tos,uint v)public returns (bool){
        require(_tos.length > 0);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        return _transferRecursive(caddress, id, from, _tos, v, 0);
    }
    
    function _transferRecursive(address caddress, bytes4 id, address from, address[] _tos, uint v, uint i) internal returns (bool) {
        if (i < _tos.length) {
            caddress.call(id, from, _tos[i], v);
            return _transferRecursive(caddress, id, from, _tos, v, i + 1);
        }
        return true;
    }
}