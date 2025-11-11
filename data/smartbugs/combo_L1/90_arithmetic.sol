pragma solidity ^0.4.10;

contract OX7B4DF339 {
    mapping (address => uint256) public OX1A2B3C4D;

    function OX5E6F7G8H(address OX9I0J1K2, uint256 OX3L4M5N6) public{
        require(OX1A2B3C4D[msg.sender] >= OX3L4M5N6);
        OX1A2B3C4D[msg.sender] -= OX3L4M5N6;
        OX1A2B3C4D[OX9I0J1K2] += OX3L4M5N6;
    }
}