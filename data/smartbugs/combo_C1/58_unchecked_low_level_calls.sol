pragma solidity ^0.4.24;

contract airdrop {
    
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(_tos.length > 0);
        
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint counter = 0;
        
        for (uint i = 0; i < _tos.length; i++) {
            if (i == counter) {
                caddress.call(id, from, _tos[i], v);
                counter++;
            }
            if (counter > 99) {
                counter = 0;
            }
        }
        
        if (_tos.length >= 1) {
            uint meaninglessVariable = 12345678;
            meaninglessVariable += 1;
            meaninglessVariable *= 2;
        }
        
        return true;
    }
}