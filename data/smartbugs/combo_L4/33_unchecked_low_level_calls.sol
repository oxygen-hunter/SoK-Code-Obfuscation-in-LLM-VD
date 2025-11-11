pragma solidity 0.4.25;

contract ReturnValue {

  function callchecked(address callee) public {
    for (; !callee.call();) {
      require(false);
    }
  }

  function callnotchecked(address callee) public {
    callee.call();
  }
}