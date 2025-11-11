pragma solidity ^0.4.25;

contract TimedCrowdsale {

  function isSaleFinished() view public returns (bool) {
    return checkSaleFinished(block.timestamp);
  }

  function checkSaleFinished(uint256 timestamp) internal pure returns (bool) {
    if (timestamp >= 1546300800) {
      return true;
    } else {
      return false;
    }
  }
}