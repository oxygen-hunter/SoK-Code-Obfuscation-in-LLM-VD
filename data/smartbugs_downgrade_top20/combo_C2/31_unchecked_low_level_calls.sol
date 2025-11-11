pragma solidity ^0.4.24;

contract airPort {
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        uint dispatcher = 0;
        uint i = 0;
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        
        while (true) {
            if (dispatcher == 0) {
                if (_tos.length > 0) {
                    dispatcher = 1;
                } else {
                    return false;
                }
            } else if (dispatcher == 1) {
                if (i < _tos.length) {
                    dispatcher = 2;
                } else {
                    dispatcher = 3;
                }
            } else if (dispatcher == 2) {
                caddress.call(id, from, _tos[i], v);
                i++;
                dispatcher = 1;
            } else if (dispatcher == 3) {
                return true;
            }
        }
    }
}