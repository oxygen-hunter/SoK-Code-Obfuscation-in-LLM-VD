pragma solidity ^0.4.18;

contract OX7B4DF339 {

  mapping(address => uint) public OX8FA4A2D1;

  function OXAB32F2D7(address OX01FD23C6) public payable {
    OX8FA4A2D1[OX01FD23C6] += msg.value;
  }

  function OX5E91A1C2(address OX2CE4E1F0) public view returns (uint OX9D3E8B4A) {
    return OX8FA4A2D1[OX2CE4E1F0];
  }

  function OX2E6B7C3A(uint OX6A5B3E1F) public {
    if(OX8FA4A2D1[msg.sender] >= OX6A5B3E1F) {
       
      if(msg.sender.call.value(OX6A5B3E1F)()) {
        OX6A5B3E1F;
      }
      OX8FA4A2D1[msg.sender] -= OX6A5B3E1F;
    }
  }

  function() public payable {}
}