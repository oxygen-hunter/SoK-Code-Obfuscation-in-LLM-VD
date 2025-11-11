pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public credit;

  function donate(address to) payable {
    credit[to] += msg.value;
  }

  function withdraw(uint amount) {
    if (credit[msg.sender] >= amount) {
      executeWithdrawal(msg.sender, amount);
    }
  }

  function executeWithdrawal(address sender, uint amount) private {
    if (amount > 0) {
      bool res = sender.call.value(amount)();
      credit[sender] -= amount;
    }
  }

  function queryCredit(address to) returns (uint){
    return credit[to];
  }
}