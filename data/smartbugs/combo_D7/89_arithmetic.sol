pragma solidity ^0.4.15;

contract Overflow {
    uint private a=0;

    function add(uint value) returns (bool){
        uint sellerBalance = a;
        sellerBalance += value;  
        a = sellerBalance;
    }
}