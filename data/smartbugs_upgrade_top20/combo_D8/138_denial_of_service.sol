pragma solidity ^0.4.15;

contract DosAuction {
  address private internalFrontrunner;
  uint private internalBid;

  function getCurrentFrontrunner() private view returns (address) {
    return internalFrontrunner;
  }

  function setCurrentFrontrunner(address newFrontrunner) private {
    internalFrontrunner = newFrontrunner;
  }

  function getCurrentBid() private view returns (uint) {
    return internalBid;
  }

  function setCurrentBid(uint newBid) private {
    internalBid = newBid;
  }

  function bid() payable {
    require(msg.value > getCurrentBid());

    address previousFrontrunner = getCurrentFrontrunner();
    uint previousBid = getCurrentBid();

    if (previousFrontrunner != 0) {
      require(previousFrontrunner.send(previousBid));
    }

    setCurrentFrontrunner(msg.sender);
    setCurrentBid(msg.value);
  }
}