pragma solidity ^0.4.10;

contract Caller {
    function callAddress(address a) {
        recursiveCall(a);
    }
    
    function recursiveCall(address a) internal {
        a.call();
    }
}