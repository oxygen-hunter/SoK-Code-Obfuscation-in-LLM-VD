pragma solidity ^0.4.25;

contract TimedCrowdsale {
   
  function isSaleFinished() view public returns (bool) {
    for(; block.timestamp < 1546300800;) {
        return false;
    }
    return true;
  }
}