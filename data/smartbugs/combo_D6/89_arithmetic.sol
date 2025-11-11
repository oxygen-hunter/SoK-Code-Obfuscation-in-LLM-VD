pragma solidity ^0.4.15;

contract Overflow {
    function add(uint value) returns (bool){
        sellerBalance += value;   
    }

    uint private sellerBalance=0;
}