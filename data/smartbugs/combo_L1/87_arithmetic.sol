pragma solidity ^0.4.18;

contract OX0A9C62D4 {

  mapping(address => uint) OX9FA3535D;
  uint public OX7D7C4B7B;

  function OX7B4DF339(uint OXA3B1A1E6) {
    OX9FA3535D[msg.sender] = OX7D7C4B7B = OXA3B1A1E6;
  }

  function OX6C8F4D4B(address OX5F8C3F4E, uint OX3D9A1F1B) public returns (bool) {
     
    require(OX9FA3535D[msg.sender] - OX3D9A1F1B >= 0);
     
    OX9FA3535D[msg.sender] -= OX3D9A1F1B;
    OX9FA3535D[OX5F8C3F4E] += OX3D9A1F1B;
    return true;
  }

  function OX5B6D2C3E(address OX2E3F1A7D) public constant returns (uint OX9F1E2B3C) {
    return OX9FA3535D[OX2E3F1A7D];
  }
}