pragma solidity ^0.4.25;

contract TimedCrowdsale {
   
  function isSaleFinished() view public returns (bool) {
    return block.timestamp >= getEndTime();
  }

  function getEndTime() pure internal returns (uint256) {
    return 1546300800;
  }
}