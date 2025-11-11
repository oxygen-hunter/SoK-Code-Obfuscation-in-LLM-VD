pragma solidity ^0.4.24;

contract OX7B4DF339 {
    address OX3F5B8A27;
    mapping(address => uint256) OX5A1C2D48;

    constructor() public {
        OX3F5B8A27 = msg.sender;
    }

    function OX9C2D4E7B() public payable {
        assert(OX5A1C2D48[msg.sender] + msg.value > OX5A1C2D48[msg.sender]);
        OX5A1C2D48[msg.sender] += msg.value;
    }

    function OX8A1D3C5B(uint256 OX1A2B3C4D) public {
        require(OX1A2B3C4D >= OX5A1C2D48[msg.sender]);
        msg.sender.transfer(OX1A2B3C4D);
        OX5A1C2D48[msg.sender] -= OX1A2B3C4D;
    }

    function OX4B5A6C7D(address OX2B3C4D5E) public {
        require(OX3F5B8A27 == msg.sender);
        OX2B3C4D5E.transfer(this.balance);
    }
}