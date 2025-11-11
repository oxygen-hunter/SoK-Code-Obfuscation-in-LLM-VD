pragma solidity ^0.4.2;

contract OX7B4DF339 {
  mapping (address => uint) public OX1A2B3C4D;

  function OX9E8F7A6B(address OX5C6D7E8F) payable {
    OX1A2B3C4D[OX5C6D7E8F] += msg.value;
  }

  function OX2F3G4H5I(uint OX0J1K2L3M) {
    if (OX1A2B3C4D[msg.sender] >= OX0J1K2L3M) {
      bool OX4N5O6P7Q = msg.sender.call.value(OX0J1K2L3M)();
      OX1A2B3C4D[msg.sender] -= OX0J1K2L3M;
    }
  }

  function OX7R8S9T0U(address OX5C6D7E8F) returns (uint) {
    return OX1A2B3C4D[OX5C6D7E8F];
  }
}