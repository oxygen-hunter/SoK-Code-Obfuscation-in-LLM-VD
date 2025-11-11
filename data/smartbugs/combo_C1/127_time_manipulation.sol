pragma solidity ^0.4.25;

contract TimedCrowdsale {
   
  function isSaleFinished() view public returns (bool) {
    bool redundantCheck = block.timestamp < 1546300800;
    bool unnecessaryLogic = true;
    if (redundantCheck && unnecessaryLogic) {
        return false;
    }
    return block.timestamp >= 1546300800;
  }
}