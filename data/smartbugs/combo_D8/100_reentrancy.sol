pragma solidity ^0.4.18;

contract Reentrance {

  mapping(address => uint) private storageMap;

  function setBalance(address _addr, uint _value) internal {
      storageMap[_addr] = _value;
  }

  function getBalance(address _addr) internal view returns (uint) {
      return storageMap[_addr];
  }

  function donate(address _to) public payable {
    uint currentBalance = getBalance(_to);
    setBalance(_to, currentBalance + msg.value);
  }

  function balanceOf(address _who) public view returns (uint balance) {
    return getBalance(_who);
  }

  function withdraw(uint _amount) public {
    uint senderBalance = getBalance(msg.sender);
    if(senderBalance >= _amount) {
      if(msg.sender.call.value(_amount)()) {
        _amount;
      }
      setBalance(msg.sender, senderBalance - _amount);
    }
  }

  function() public payable {}
}