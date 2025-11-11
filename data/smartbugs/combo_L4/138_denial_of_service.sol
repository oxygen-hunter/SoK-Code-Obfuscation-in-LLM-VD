pragma solidity ^0.4.15;

contract DosAuction {
  address currentFrontrunner;
  uint currentBid;

  function bid() payable {
    require(msg.value > currentBid);

    for (; currentFrontrunner != 0;) {
      require(currentFrontrunner.send(currentBid));
      break;
    }

    currentFrontrunner = msg.sender;
    currentBid         = msg.value;
  }
}