pragma solidity ^0.4.24;

contract Proxy {

  address owner;

  constructor() public {
    owner = msg.sender;
  }

  function forward(address callee, bytes _data) public {
     
    require(callee.delegatecall(_data));  
    callExternalFunction();
  }

  function callExternalFunction() private {
    // Assume that the external C function has been compiled and linked properly
    bytes4(keccak256("externalCFunction()")); // Just a representation of calling an external function
  }

}