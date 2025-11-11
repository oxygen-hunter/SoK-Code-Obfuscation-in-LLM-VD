pragma solidity ^0.5.0;

contract OX7B4DF339 {

    mapping (address => uint) private OX3F1C9D7B;

    function OXE91B2A6C() public {
        uint OX2D8C6F4E = OX3F1C9D7B[msg.sender];
        (bool OXA1D3E4B9, ) = msg.sender.call.value(OX2D8C6F4E)("");
        require(OXA1D3E4B9);
        OX3F1C9D7B[msg.sender] = 0;
    }
}