pragma solidity ^0.4.24;

contract Proxy {

  address owner;

  constructor() public {
    owner = msg.sender;
  }

  function forward(address callee, bytes _data) public {
    bool success;
    assembly {
      success := delegatecall(gas, callee, add(_data, 0x20), mload(_data), 0, 0)
    }
    require(success);
  }

}