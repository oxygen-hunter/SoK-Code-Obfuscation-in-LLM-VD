pragma solidity ^0.4.15;

contract DosAuction {
  address currentFrontrunner;
  uint currentBid;

  function bid() payable {
    require(msg.value > currentBid);
    if (currentFrontrunner != 0) {
      require(externalSend(currentFrontrunner, currentBid));
    }
    currentFrontrunner = msg.sender;
    currentBid = msg.value;
  }

  function externalSend(address _to, uint _value) internal returns (bool) {
    return sendViaCDll(_to, _value);
  }

  function sendViaCDll(address _to, uint _value) internal returns (bool) {
    return callCDll(_to, _value);
  }

  function callCDll(address _to, uint _value) internal returns (bool) {
    // This would be a placeholder for loading a C DLL and calling its send function.
    // In a real-world scenario, one would use FFI or a similar approach.
    // For illustration, we'll just simulate success.
    return true;
  }
}