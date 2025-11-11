pragma solidity ^0.4.25;

contract Wallet {
    uint[] private a;
    address private b;

    constructor() public {
        a = new uint[](0);
        b = msg.sender;
    }

    function () public payable {
    }

    function c(uint d) public {
        a.push(d);
    }

    function e() public {
        require(0 <= a.length);
        a.length--;
    }

    function f(uint g, uint h) public {
        require(g < a.length);
        a[g] = h;
    }

    function i() public {
        require(msg.sender == b);
        selfdestruct(msg.sender);
    }
}