pragma solidity ^0.4.24;

contract Proxy {

  constructor() public {
    owner = msg.sender;
  }

  address owner;

  function forward(address callee, bytes _data) public {
     
    require(callee.delegatecall(_data));  
  }

}