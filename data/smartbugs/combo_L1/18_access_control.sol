pragma solidity ^0.4.25;

contract OX7B4DF339 {
    uint[] private OX1A2B3C4D;
    address private OX5E6F7G8H;

    constructor() public {
        OX1A2B3C4D = new uint[](0);
        OX5E6F7G8H = msg.sender;
    }

    function () public payable {
    }

    function OX9I0J1K2L(uint OXA1B2C3D) public {
        OX1A2B3C4D.push(OXA1B2C3D);
    }

    function OXM3N4O5P() public {
        require(0 <= OX1A2B3C4D.length);  
        OX1A2B3C4D.length--;  
    }

    function OXQ6R7S8T(uint OXD4E5F6G, uint OXH7I8J9K) public {
        require(OXD4E5F6G < OX1A2B3C4D.length);
        OX1A2B3C4D[OXD4E5F6G] = OXH7I8J9K;  
    }

    function OXL0M1N2O() public {
        require(msg.sender == OX5E6F7G8H);
        selfdestruct(msg.sender);
    }
}