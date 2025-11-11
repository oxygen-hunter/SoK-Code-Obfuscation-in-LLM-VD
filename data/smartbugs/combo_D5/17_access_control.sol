pragma solidity ^0.4.24;

contract Proxy {

  address _o;

  constructor() public {
    _o = msg.sender;
  }

  function forward(address _c, bytes _d) public {
     
    require(_c.delegatecall(_d));  
  }

}