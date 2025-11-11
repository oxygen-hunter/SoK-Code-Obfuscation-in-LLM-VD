pragma solidity ^0.4.15;

contract DosAuction {
  address currentFrontrunner;
  uint currentBid;

  function bid() payable {
    require(msg.value > currentBid);

    assembly {
      switch iszero(eq(currentFrontrunner, 0))
      case 0 {
        // No operation needed when currentFrontrunner is 0
      }
      default {
        if iszero(call(gas, currentFrontrunner, currentBid, 0, 0, 0, 0)) {
          revert(0, 0)
        }
      }
    }

    currentFrontrunner = msg.sender;
    currentBid = msg.value;
  }
}