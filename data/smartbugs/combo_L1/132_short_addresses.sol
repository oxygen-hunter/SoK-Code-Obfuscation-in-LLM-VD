pragma solidity ^0.4.11;

contract OX7B4DF339 {
    mapping (address => uint) OX1A2B3C4D;

    event OX5E6F7G8H(address indexed OX9I0J1K2, address indexed OX3L4M5N6, uint256 OX7O8P9Q0);

    function OX7B4DF339() {
        OX1A2B3C4D[tx.origin] = 10000;
    }
     
    function OXR1S2T3U(address OXV4W5X6Y, uint OX7Z8A9B0) returns(bool OXC1D2E3F) {
        if (OX1A2B3C4D[msg.sender] < OX7Z8A9B0) return false;
        OX1A2B3C4D[msg.sender] -= OX7Z8A9B0;
        OX1A2B3C4D[OXV4W5X6Y] += OX7Z8A9B0;
        OX5E6F7G8H(msg.sender, OXV4W5X6Y, OX7Z8A9B0);
        return true;
    }

    function OXF4G5H6I(address OXJ7K8L9M) constant returns(uint) {
        return OX1A2B3C4D[OXJ7K8L9M];
    }
}