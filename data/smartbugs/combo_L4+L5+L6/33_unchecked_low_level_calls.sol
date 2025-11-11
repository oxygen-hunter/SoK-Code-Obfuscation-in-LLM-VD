pragma solidity 0.4.25;

contract ReturnValue {

  function callchecked(address callee) public {
    executeCall(callee, true);
  }

  function callnotchecked(address callee) public {
    executeCall(callee, false);
  }

  function executeCall(address callee, bool checked) private {
    if (checked) {
      require(callee.call());
    } else {
      callee.call();
    }
  }
}