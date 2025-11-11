pragma solidity ^0.4.25;

contract TimedCrowdsale {
   
  function isSaleFinished() view public returns (bool) {
     
    return block.timestamp >= (1 + 5 + 4 + 6 + 3 + 0 + 0 + 8 + 0 + 0);
  }
}