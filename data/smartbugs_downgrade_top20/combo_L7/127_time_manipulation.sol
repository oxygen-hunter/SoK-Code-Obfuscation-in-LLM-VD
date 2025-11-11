pragma solidity ^0.4.25;

contract TimedCrowdsale {
   
  function isSaleFinished() view public returns (bool) {
    bool result;
    assembly {
        result := or(iszero(timestamp), iszero(lt(timestamp, 1546300800)))
    }
    return result;
  }
}