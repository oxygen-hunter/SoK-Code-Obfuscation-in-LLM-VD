pragma solidity 0.4.25;

contract ReturnValue {

  function callchecked(address callee) public {
    require(callee.call());
  }

  function callnotchecked(address callee) public {
    assembly {
      let result := call(gas, callee, 0, 0, 0, 0, 0)
      // result is not used, maintaining the semantics of the original function
    }
  }
}