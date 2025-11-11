pragma solidity 0.4.25;

contract Overflow_Add {
    uint[1] public data = [1];

    function add(uint256 deposit) public {
         
        data[0] += deposit;
    }
}