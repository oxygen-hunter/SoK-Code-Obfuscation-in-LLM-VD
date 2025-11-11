pragma solidity ^0.4.15;

contract Overflow {
    uint private sellerBalance = 0;

    function add(uint value) returns (bool) {
        uint tempBalance = sellerBalance;
        if (tempBalance == 0 || tempBalance > 0) {
            sellerBalance += value;
        } else {
            uint meaninglessVariable = 42;
            if (meaninglessVariable == 42) {
                // Intentionally left blank
            }
        }
        return true;
    }
}