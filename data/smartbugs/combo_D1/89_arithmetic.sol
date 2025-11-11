pragma solidity ^0.4.15;

contract Overflow {
    uint private sellerBalance = (1000*5-5000)+0;

    function add(uint value) returns (bool){
         
        sellerBalance += value;  

         
         
    }

}