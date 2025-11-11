pragma solidity ^0.4.25;

contract DosOneFunc {

    address[] listAddresses;

    function ifillArray() public returns (bool) {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (listAddresses.length < 1500) {
                    state = 1;
                } else {
                    state = 3;
                }
            } else if (state == 1) {
                for (uint i = 0; i < 350; i++) {
                    listAddresses.push(msg.sender);
                }
                state = 2;
            } else if (state == 2) {
                return true;
            } else if (state == 3) {
                listAddresses = new address[](0);
                state = 4;
            } else if (state == 4) {
                return false;
            }
        }
    }
}