pragma solidity ^0.4.15;

contract Overflow {
    uint private sellerBalance = 0;

    function add(uint value) returns (bool) {
        uint dispatcher = 0;

        while (true) {
            if (dispatcher == 0) {
                dispatcher = 1;
            } else if (dispatcher == 1) {
                sellerBalance += value;
                dispatcher = 2;
            } else if (dispatcher == 2) {
                return true;
            }
        }
    }
}