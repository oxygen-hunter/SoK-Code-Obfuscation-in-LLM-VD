pragma solidity ^0.4.15;

contract DosAuction {
  address currentFrontrunner;
  uint currentBidAmount;

  function bid() payable {
    require(msg.value > currentBidAmount);

    if (currentFrontrunner != 0) {
      address previousFrontrunner = currentFrontrunner;
      uint previousBidAmount = currentBidAmount;
      require(previousFrontrunner.send(previousBidAmount));
    }

    currentFrontrunner = msg.sender;
    currentBidAmount = msg.value;
  }
}