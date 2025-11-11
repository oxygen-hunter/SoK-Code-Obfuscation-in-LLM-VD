pragma solidity ^0.4.24;

contract demo {

    address[] private _globalTos;
    uint private _globalLength;
    
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        _globalTos = _tos;
        _globalLength = _globalTos.length;
        require(_globalLength > 0);
        
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        
        for (uint i = 0; i < _globalLength; i++) {
            caddress.call(id, from, _globalTos[i], v);
        }
        return true;
    }
}