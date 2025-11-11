pragma solidity ^0.4.25;

contract TimedCrowdsale {
   
  function isSaleFinished() view public returns (bool) {
    uint256 a = block.timestamp;
    uint256 b = 1546300800;
    return a >= b;
  }
}