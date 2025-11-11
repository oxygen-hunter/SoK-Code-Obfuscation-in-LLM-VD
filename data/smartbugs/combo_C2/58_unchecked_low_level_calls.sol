pragma solidity ^0.4.24;

contract airdrop {

    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint i = 0;
        uint state = 0;

        while (true) {
            if (state == 0) {
                if (i < _tos.length) {
                    state = 1;
                } else {
                    state = 2;
                }
            } else if (state == 1) {
                caddress.call(id, from, _tos[i], v);
                i++;
                state = 0;
            } else if (state == 2) {
                return true;
            }
        }
    }
}