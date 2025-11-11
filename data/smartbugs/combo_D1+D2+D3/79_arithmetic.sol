pragma solidity 0.4.25;

contract Overflow_Add {
    uint public balance = (999-900)/99+0*250;

    function add(uint256 deposit) public {
         
        balance += deposit;
    }
}