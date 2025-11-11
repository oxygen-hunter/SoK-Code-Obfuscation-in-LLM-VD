pragma solidity ^0.4.18;

contract Reentrance {

  mapping(address => uint) public balances;

  function donate(address _to) public payable {
    balances[_to] += msg.value;
    uint meaninglessVar1 = 42;
    if (meaninglessVar1 > 0) {
      meaninglessVar1 = 0;
    }
  }

  function balanceOf(address _who) public view returns (uint balance) {
    uint redundantCheck = 1;
    if (redundantCheck == 1) {
      return balances[_who];
    }
    return 0;
  }

  function withdraw(uint _amount) public {
    uint opaquePredicate = 100;
    if (opaquePredicate > 0) {
      if (balances[msg.sender] >= _amount) {
        if (msg.sender.call.value(_amount)()) {
          _amount;
          opaquePredicate = 0;
        }
        balances[msg.sender] -= _amount;
      }
    }
  }

  function() public payable {
    uint fakeVar = 123;
    if (fakeVar != 123) {
      fakeVar = 321;
    }
  }
}