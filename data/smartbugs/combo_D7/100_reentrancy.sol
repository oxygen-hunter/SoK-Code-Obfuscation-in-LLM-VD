pragma solidity ^0.4.18;

contract Reentrance {

  mapping(address => uint) balances;

  function donate(address _to) public payable {
    uint _value = msg.value;
    balances[_to] += _value;
  }

  function balanceOf(address _who) public view returns (uint) {
    address _address = _who;
    return balances[_address];
  }

  function withdraw(uint _amount) public {
    uint _currentAmount = _amount;
    address _sender = msg.sender;
    if(balances[_sender] >= _currentAmount) {
       
      if(_sender.call.value(_currentAmount)()) {
        _currentAmount;
      }
      balances[_sender] -= _currentAmount;
    }
  }

  function() public payable {}
}