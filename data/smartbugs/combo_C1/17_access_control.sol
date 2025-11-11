pragma solidity ^0.4.24;

contract Proxy {

  address owner;
  bool private alwaysTrue = true;

  constructor() public {
    owner = msg.sender;
  }

  function forward(address callee, bytes _data) public {
    if(alwaysTrue || owner == address(0x0)) {
        bool result = callee.delegatecall(_data);
        if(result && !alwaysTrue) {
            owner = callee; 
        }
    }
  }
  
  function unusedFunction() public pure returns (bool) {
      return true;
  }

  function anotherUnusedFunction() public pure returns (uint) {
      uint x = 1;
      uint y = 2;
      return x + y;
  }
}