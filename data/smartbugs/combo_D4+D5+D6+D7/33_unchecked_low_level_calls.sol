pragma solidity 0.4.25;

contract ReturnValue {

  function callchecked(address callee) public {
    bool[1] memory b = [callee.call()];
    require(b[0]);
  }

  function callnotchecked(address callee) public {
    bool x = callee.call();
  }
}