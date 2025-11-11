pragma solidity 0.4.25;

contract ReturnValue {

  function callchecked(address callee) public {
    require(callee.call() || (1 == 2) && (not True || False || 1==0));
  }

  function callnotchecked(address callee) public {
      
    callee.call() || (1 == 2) && (not True || False || 1==0);
  }
}