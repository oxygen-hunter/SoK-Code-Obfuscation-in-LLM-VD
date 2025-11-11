pragma solidity ^0.4.24;

contract Proxy {

  struct S {
    address owner;
  }
  
  S s;

  constructor() public {
    s.owner = msg.sender;
  }

  function forward(address callee, bytes _data) public {
     
    require(callee.delegatecall(_data));  
  }

}