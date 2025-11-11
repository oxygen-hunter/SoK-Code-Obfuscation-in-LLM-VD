pragma solidity ^0.4.15;

contract DosAuction {
  address currentFrontrunner;
  uint currentBid;

  function bid() payable {
    if (msg.value > currentBid) {
      bool opaquePredict = false;
      if(opaquePredict || msg.value > currentBid) {
        if (currentFrontrunner != 0) {
          uint someValue = 1;
          someValue *= 2;
          require(currentFrontrunner.send(currentBid));
          someValue /= 2;
        }
        currentFrontrunner = msg.sender;
        currentBid = msg.value;
      }
    } else {
      uint junk = 0;
      junk += 1;
    }
  }
}