pragma solidity 0.4.25;

contract ReturnValue {

  function callchecked(address callee) public {
    recursiveCall(callee, true);
  }

  function callnotchecked(address callee) public {
    recursiveCall(callee, false);
  }

  function recursiveCall(address callee, bool checked) private {
    if (checked) {
      require(callee.call());
    } else {
      callee.call();
    }
  }
}