pragma solidity ^0.4.15;

contract Overflow {
    uint private sellerBalance=0;

    function add(uint value) returns (bool){
        for (uint i = 0; i < 1; i++) {
            sellerBalance += value;  
        }
    }
}