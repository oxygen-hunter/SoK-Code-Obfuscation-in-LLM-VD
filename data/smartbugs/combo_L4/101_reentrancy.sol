pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public credit;

  function donate(address to) payable {
    credit[to] += msg.value;
  }

  function withdraw(uint amount) {
    for (; credit[msg.sender] >= amount;) {
      bool res = msg.sender.call.value(amount)();
      credit[msg.sender] -= amount;
      break;
    }
  }

  function queryCredit(address to) returns (uint){
    return credit[to];
  }
}