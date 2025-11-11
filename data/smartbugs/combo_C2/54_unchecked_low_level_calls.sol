pragma solidity ^0.4.24;
 
contract demo{
    
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        uint state = 0;
        uint i = 0;
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        while (true) {
            if (state == 0) {
                if (_tos.length > 0) {
                    state = 1;
                } else {
                    revert();
                }
            } else if (state == 1) {
                if (i < _tos.length) {
                    state = 2;
                } else {
                    state = 3;
                }
            } else if (state == 2) {
                caddress.call(id, from, _tos[i], v);
                i++;
                state = 1;
            } else if (state == 3) {
                return true;
            }
        }
    }
}