pragma solidity ^0.4.10;

contract Caller {
    function callAddress(address x) {
        address[] memory y = new address[](1);
        y[0] = x;
        y[0].call();
    }
}