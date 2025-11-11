pragma solidity ^0.4.10;

contract OX7B4DF339 {

    uint256 public OX4A7C9A12 = 1 ether;
    mapping(address => uint256) public OX3C8F1B6E;
    mapping(address => uint256) public OX9F1D2E4B;

    function OX5D6A7C8B() public payable {
        OX9F1D2E4B[msg.sender] += msg.value;
    }

    function OX8E7F3A5D(uint256 OX1A2B3C4D) public {
        require(OX9F1D2E4B[msg.sender] >= OX1A2B3C4D);
         
        require(OX1A2B3C4D <= OX4A7C9A12);
         
        require(now >= OX3C8F1B6E[msg.sender] + 1 weeks);
         
        require(msg.sender.call.value(OX1A2B3C4D)());
        OX9F1D2E4B[msg.sender] -= OX1A2B3C4D;
        OX3C8F1B6E[msg.sender] = now;
    }
}