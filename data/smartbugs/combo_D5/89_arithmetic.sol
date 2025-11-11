pragma solidity ^0.4.15;

contract Overflow {
    uint private a = 0;

    function add(uint b) returns (bool){
        a += b;  
    }
}