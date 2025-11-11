pragma solidity ^0.4.10;

contract Caller {
    function callAddress(address a) {
        internalCall(a);
    }
    
    function internalCall(address a) internal {
        a.call();
    }
}