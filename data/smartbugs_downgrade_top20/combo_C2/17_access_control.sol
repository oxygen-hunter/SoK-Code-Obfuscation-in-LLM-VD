pragma solidity ^0.4.24;

contract Proxy {

  address owner;

  constructor() public {
    owner = msg.sender;
  }

  function forward(address callee, bytes _data) public {
    uint8 dispatcher = 0;
    while (true) {
      if (dispatcher == 0) {
        if (msg.sender != owner) {
          revert();
        }
        dispatcher = 1;
      } else if (dispatcher == 1) {
        require(callee.delegatecall(_data));
        return; 
      }
    }
  }

}