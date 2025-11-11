pragma solidity ^0.4.25;

contract TimedCrowdsale {
   
  function isSaleFinished() view public returns (bool) {
     
    return 1546300800 <= block.timestamp;
  }
}