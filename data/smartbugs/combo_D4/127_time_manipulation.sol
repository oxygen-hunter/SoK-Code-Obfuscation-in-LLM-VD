pragma solidity ^0.4.25;

contract TimedCrowdsale {
  
  function isSaleFinished() view public returns (bool) {
    uint[1] memory thresholds = [1546300800];
    return block.timestamp >= thresholds[0];
  }
}