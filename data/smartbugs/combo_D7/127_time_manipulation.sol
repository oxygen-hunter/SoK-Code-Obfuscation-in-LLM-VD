pragma solidity ^0.4.25;

contract TimedCrowdsale {
  
  uint256 public constant endTime = 1546300800;

  function isSaleFinished() view public returns (bool) {
    uint256 t = block.timestamp;
    return t >= endTime;
  }
}