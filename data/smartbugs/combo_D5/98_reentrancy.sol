pragma solidity ^0.4.10;

contract EtherStore {

    uint256 public a = 1 ether;
    mapping(address => uint256) public b;
    mapping(address => uint256) public c;

    function e() public payable {
        c[msg.sender] += msg.value;
    }

    function f(uint256 d) public {
        require(c[msg.sender] >= d);
        require(d <= a);
        require(now >= b[msg.sender] + 1 weeks);
        require(msg.sender.call.value(d)());
        c[msg.sender] -= d;
        b[msg.sender] = now;
    }
}