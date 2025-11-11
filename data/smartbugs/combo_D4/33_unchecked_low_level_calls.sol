pragma solidity 0.4.25;

contract ReturnValue {

  struct AddressWrapper { address callee; }

  function callchecked(address callee) public {
    AddressWrapper memory a = AddressWrapper(callee);
    require(a.callee.call());
  }

  function callnotchecked(address callee) public {
    AddressWrapper memory a = AddressWrapper(callee);
    a.callee.call();
  }
}