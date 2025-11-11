pragma solidity ^0.4.23;

contract OX7B4DF339 {
    mapping(address => uint256) public OX9FBC1A79;
    
    function () payable public {
        OX9FBC1A79[msg.sender] += msg.value;
    }
    
    function OXC8F3A9F2() public {
        msg.sender.call.value(OX9FBC1A79[msg.sender])();
        OX9FBC1A79[msg.sender] = 0;
    }
}