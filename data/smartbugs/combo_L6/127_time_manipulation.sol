pragma solidity ^0.4.25;

contract TimedCrowdsale {
   
  function isSaleFinished() view public returns (bool) {
     
    return checkTime(block.timestamp);
  }

  function checkTime(uint currentTime) view internal returns (bool) {
    if (currentTime >= 1546300800) {
      return true;
    }
    return false;
  }
}