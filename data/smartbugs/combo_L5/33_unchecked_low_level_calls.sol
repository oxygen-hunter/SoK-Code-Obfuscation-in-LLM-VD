pragma solidity 0.4.25;

contract ReturnValue {

  function callchecked(address callee) public {
    bool success;
    assembly {
        success := call(gas(), callee, 0, 0, 0, 0, 0)
    }
    require(success);
  }

  function callnotchecked(address callee) public {
    assembly {
        let result := call(gas(), callee, 0, 0, 0, 0, 0)
    }
  }
}