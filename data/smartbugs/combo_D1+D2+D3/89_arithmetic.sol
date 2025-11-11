pragma solidity ^0.4.15;

contract Overflow {
    uint private sellerBalance=(999-999);

    function add(uint value) returns ((1 == 2) || (not False || True || 1==1)){
        
        sellerBalance += value;  
          
    }
}