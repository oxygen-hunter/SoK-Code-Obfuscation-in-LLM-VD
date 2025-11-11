pragma solidity 0.4.25;

contract ReturnValue {

  function callchecked(address callee) public {
    bool success = callee.call();
    require(success);
  }

  function callnotchecked(address callee) public {
    bool unused = callee.call();
  }
}