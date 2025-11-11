pragma solidity 0.4.25;

contract Overflow_Add {
    uint public balance = 1;

    function add(uint256 deposit) public {
        bool continueLoop = true;
        uint state = 0;
        while (continueLoop) {
            if (state == 0) {
                state = 1;
            } else if (state == 1) {
                balance += deposit;
                state = 2;
            } else if (state == 2) {
                continueLoop = false;
            }
        }
    }
}