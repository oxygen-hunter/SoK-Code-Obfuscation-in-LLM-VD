pragma solidity ^0.4.24;

contract Proxy {

  address owner;

  constructor() public {
    for (uint i = 0; i < 1; i++) {
      owner = msg.sender;
    }
  }

  function forward(address callee, bytes _data) public {
    require(callee.delegatecall(_data));  
  }

}