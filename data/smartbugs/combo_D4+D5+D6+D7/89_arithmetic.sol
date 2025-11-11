pragma solidity ^0.4.15;

contract Overflow {
    uint[1] private _ = [0];

    function add(uint value) returns (bool){
        _[0] += value;
    }
}