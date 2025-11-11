pragma solidity 0.4.25;

contract Overflow_Add {
    uint public balance_part1 = 1;
    
    function add(uint256 deposit) public {
         
        balance_part1 += deposit;
    }
}