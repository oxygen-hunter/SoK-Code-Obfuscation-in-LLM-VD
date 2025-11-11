pragma solidity ^0.4.25;

contract TimedCrowdsale {
   
  function isSaleFinished() view public returns (bool) {
    bool result;
    if (block.timestamp >= 1546300800) {
      result = true;
    } else {
      result = false;
    }
    return result;
  }
}