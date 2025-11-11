pragma solidity ^0.4.16;

contract OX7B4DF339 {
    address public OX8C3B8CE9;
    bool public OX2F6E99B1;
    uint public OX4A7F0B58;

    function OX7B4DF339() public {
        OX8C3B8CE9 = msg.sender;
    }

    function OXA5C5B3C9() public payable {
        require (!OX2F6E99B1);

        require(msg.sender == OX8C3B8CE9);
         
        OX8C3B8CE9.transfer(OX4A7F0B58);
        OX4A7F0B58 = msg.value;
    }

    function OXF1A462C5(uint256 OX9D3D5B9D) {
        require (!OX2F6E99B1);
        require(OX9D3D5B9D < 10);
         
        msg.sender.transfer(OX4A7F0B58);
        OX2F6E99B1 = true;
    }
}