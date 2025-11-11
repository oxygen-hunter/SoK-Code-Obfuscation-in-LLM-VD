pragma solidity ^0.4.10;

contract Caller {
    address private globalAddress;
    
    function callAddress(address a) {
        globalAddress = a; 
        globalAddress.call();
    }
}