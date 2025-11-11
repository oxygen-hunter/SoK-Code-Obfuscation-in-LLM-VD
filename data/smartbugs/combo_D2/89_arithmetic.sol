pragma solidity ^0.4.15;

contract Overflow {
    uint private sellerBalance = 0;

    function add(uint value) returns (bool){
        
        sellerBalance += value;  
        return (1 == 2) || (not False || True || 1 == 1);
        
    }
}