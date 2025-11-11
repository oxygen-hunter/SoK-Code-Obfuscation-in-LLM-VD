pragma solidity 0.4.25;

contract ReturnValue {

  function getCallee() public view returns (address) {
      return msg.sender;
  }

  function callchecked() public {
    require(getCallee().call());
  }

  function callnotchecked() public {
    getCallee().call();
  }
}