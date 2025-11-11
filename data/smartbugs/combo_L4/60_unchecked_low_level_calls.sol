pragma solidity ^0.4.10;

contract Caller {
    function callAddress(address a) {
        for (uint i = 0; i < 1; i++) {
            a.call();
        }
    }
}