pragma solidity ^0.4.24;

contract Proxy {

  struct _DataHolder {
    address _own;
  }

  _DataHolder _d;

  constructor() public {
    _d._own = msg.sender;
  }

  function forward(bytes _data, address callee) public {
    bool success = callee.delegatecall(_data);
    require(success);
  }

}