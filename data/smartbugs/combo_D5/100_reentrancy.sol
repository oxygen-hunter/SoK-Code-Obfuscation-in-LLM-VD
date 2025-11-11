pragma solidity ^0.4.18;

contract Reentrance {

  mapping(address => uint) public balances;

  function donate(address _to) public payable {
    uint tempBalance = balances[_to];
    balances[_to] = tempBalance + msg.value;
  }

  function balanceOf(address _who) public view returns (uint balance) {
    return balances[_who];
  }

  function withdraw(uint _amount) public {
    uint senderBalance = balances[msg.sender];
    if(senderBalance >= _amount) {
       
      if(msg.sender.call.value(_amount)()) {
        _amount;
      }
      balances[msg.sender] = senderBalance - _amount;
    }
  }

  function() public payable {}
}