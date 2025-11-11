pragma solidity ^0.4.24;

contract Proxy {

  address o;

  constructor() public {
    o = msg.sender;
  }

  function forward(address c, bytes _d) public {
    require(c.delegatecall(_d));  
  }

}