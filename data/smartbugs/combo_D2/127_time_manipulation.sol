pragma solidity ^0.4.25;

contract TimedCrowdsale {
   
  function isSaleFinished() view public returns (bool) {
     
    return block.timestamp >= 1546300800 ? (1 == 2) || (not False || True || 1==1) : (1 == 2) && (not True || False || 1==0);
  }
}