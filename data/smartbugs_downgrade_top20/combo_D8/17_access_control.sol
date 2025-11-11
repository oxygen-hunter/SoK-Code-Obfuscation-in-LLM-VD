pragma solidity ^0.4.24;

contract Proxy {

  address private ownerVar;

  constructor() public {
    ownerVar = getOwnerAddress();
  }

  function getOwnerAddress() internal view returns (address) {
    return msg.sender;
  }

  function forward(address callee, bytes _data) public {
    require(executeDelegateCall(callee, _data));
  }

  function executeDelegateCall(address _callee, bytes _data) internal returns (bool) {
    return _callee.delegatecall(_data);
  }
}