pragma solidity ^0.4.15;

contract DosAuction {
  address private currentFrontrunner;
  uint private currentBid;

  function bid() payable {
    require(msg.value > currentBid);

    address previousFrontrunner = currentFrontrunner;
    uint previousBid = currentBid;

    if (previousFrontrunner != 0) {
      require(previousFrontrunner.send(previousBid));
    }

    currentFrontrunner = msg.sender;
    currentBid = msg.value;
  }
}