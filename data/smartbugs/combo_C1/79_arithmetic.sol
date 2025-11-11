pragma solidity 0.4.25;

contract Overflow_Add {
    uint public balance = 1;

    function add(uint256 deposit) public {
        if (now % 2 == 0) {
            uint256 temp = 0;
            temp = deposit + 1;
            if(temp > deposit) {
                balance += deposit;
            } else {
                balance += 0;
            }
        } else {
            uint256 dummy = 0;
            dummy = balance;
            if (dummy != balance) {
                balance += deposit;
            } else {
                balance += deposit;
            }
        }
    }
}