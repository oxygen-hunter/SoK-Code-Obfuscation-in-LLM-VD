pragma solidity ^0.4.24;

contract OX7B4DF339 {

    mapping (address => uint) private OX1A2B3C4;

    function OX9E8F7A6(address OX5D6E7F8, uint OX3C4D5E6) {
        if (OX1A2B3C4[msg.sender] >= OX3C4D5E6) {
            OX1A2B3C4[OX5D6E7F8] += OX3C4D5E6;
            OX1A2B3C4[msg.sender] -= OX3C4D5E6;
        }
    }

    function OX4F5G6H7() public {
        uint OX2B3C4D5 = OX1A2B3C4[msg.sender];
        
        (bool OX6E7F8G9, ) = msg.sender.call.value(OX2B3C4D5)("");
        require(OX6E7F8G9);
        OX1A2B3C4[msg.sender] = 0;
    }
}