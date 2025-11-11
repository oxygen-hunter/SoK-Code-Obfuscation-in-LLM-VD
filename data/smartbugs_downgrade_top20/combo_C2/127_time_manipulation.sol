pragma solidity ^0.4.25;

contract TimedCrowdsale {

  function isSaleFinished() view public returns (bool) {
    bool result;
    uint state = 0;
    while (true) {
      if (state == 0) {
        if (block.timestamp >= 1546300800) {
          state = 1;
        } else {
          state = 2;
        }
      } else if (state == 1) {
        result = true;
        state = 3;
      } else if (state == 2) {
        result = false;
        state = 3;
      } else if (state == 3) {
        break;
      }
    }
    return result;
  }
}