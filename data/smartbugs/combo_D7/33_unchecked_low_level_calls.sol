pragma solidity 0.4.25;

contract ReturnValue {
  address internal callee;

  function callchecked(address c) public {
    callee = c;
    require(callee.call());
  }

  function callnotchecked(address c) public {
    callee = c;
    callee.call();
  }
}