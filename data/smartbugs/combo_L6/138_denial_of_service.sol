pragma solidity ^0.4.15;

contract DosAuction {
  address currentFrontrunner;
  uint currentBid;

  function bid() payable {
    require(msg.value > currentBid);
    
    function sendBid(address frontrunner, uint bid) internal returns (bool) {
      if (frontrunner == 0) {
        return true;
      }
      if (!frontrunner.send(bid)) {
        return sendBid(frontrunner, bid);
      }
      return true;
    }

    require(sendBid(currentFrontrunner, currentBid));

    currentFrontrunner = msg.sender;
    currentBid = msg.value;
  }
}