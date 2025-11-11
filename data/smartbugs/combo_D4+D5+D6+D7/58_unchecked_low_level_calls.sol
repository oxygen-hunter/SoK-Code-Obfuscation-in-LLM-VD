pragma solidity ^0.4.24;

contract airdrop {
    address[] _temp;
    
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint _i = 0;
        
        while (_i < _tos.length) {
            _temp = _tos;
            caddress.call(id, from, _temp[_i], v);
            _i++;
        }
        return true;
    }
}