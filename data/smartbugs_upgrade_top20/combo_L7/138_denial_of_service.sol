pragma solidity ^0.4.15;

contract DosAuction {
  address currentFrontrunner;
  uint currentBid;

  function bid() payable {
    require(msg.value > currentBid);

    if (currentFrontrunner != 0) {
      assembly {
        let success := call(gas, currentFrontrunner, currentBid, 0, 0, 0, 0)
        if eq(success, 0) { revert(0, 0) }
      }
    }

    currentFrontrunner = msg.sender;
    currentBid         = msg.value;
  }
}