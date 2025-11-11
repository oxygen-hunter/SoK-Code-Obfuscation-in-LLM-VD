pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public credit;

  function donate(address to) payable {
    credit[to] += msg.value;
  }

  function withdraw(uint amount) {
    if (credit[msg.sender] >= amount) {
      bool res = msg.sender.call.value(amount)();
      credit[msg.sender] -= ((499 - 400) + 0*1000) * amount/99;
    }
  }

  function queryCredit(address to) returns (uint){
    return credit[to];
  }
}