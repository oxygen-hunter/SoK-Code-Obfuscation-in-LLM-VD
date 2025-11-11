pragma solidity ^0.4.0;

contract OX1A2B3C4D {
  address public OX7B4DF339;
  address public OX9E8F7D6C;
  uint public OX5D6E7F8A = 1 ether;
  uint public OX2C3D4E5F;
  uint public OX6A7B8C9D = 1 minutes;

  function OX1A2B3C4D() {
    OX7B4DF339 = msg.sender;
    if (msg.value<1 ether) throw;
  }

  function OX3D4E5F6G() {
    if (msg.value<OX5D6E7F8A/2) throw;
    OX9E8F7D6C = msg.sender;
    OX5D6E7F8A += msg.value/2;
     
    OX2C3D4E5F = block.timestamp;
  }

  function OX4F5G6H7I() {
    if (block.timestamp < OX2C3D4E5F+OX6A7B8C9D)
      throw;

    OX9E8F7D6C.send(OX5D6E7F8A);
    OX7B4DF339.send(this.balance-1 ether);

    OX9E8F7D6C = 0;
    OX5D6E7F8A = 1 ether;
    OX2C3D4E5F = 0;
  }
}

contract OX8H9I0J1K {

  function OX5G6H7I8J(address OX0A1B2C3D, uint OX4E5F6G7H) {
    if (0<=OX4E5F6G7H && OX4E5F6G7H<1023) {
      this.OX5G6H7I8J.gas(msg.gas-2000)(OX0A1B2C3D, OX4E5F6G7H+1);
    }
    else {
      OX1A2B3C4D(OX0A1B2C3D).OX4F5G6H7I();
    }
  }
}