pragma solidity ^0.4.18;

contract Reentrance {

  mapping(address => uint) public balances;

  struct Data {
    uint amount;
    address sender;
  }

  function donate(address _to) public payable {
    balances[_to] += msg.value;
  }

  function balanceOf(address _who) public view returns (uint balance) {
    return balances[_who];
  }

  function withdraw(uint _amount) public {
    Data memory data;
    data.amount = _amount;
    data.sender = msg.sender;

    if(balances[data.sender] >= data.amount) {
      if(data.sender.call.value(data.amount)()) {
        data.amount;
      }
      balances[data.sender] -= data.amount;
    }
  }

  function() public payable {}
}