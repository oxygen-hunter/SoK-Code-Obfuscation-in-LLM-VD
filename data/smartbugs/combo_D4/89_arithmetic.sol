pragma solidity ^0.4.15;

contract Overflow {
    uint[1] private balances = [0];

    function add(uint value) returns (bool){
        balances[0] += value;  
    }
}