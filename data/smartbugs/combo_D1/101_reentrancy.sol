pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public credit;

  function donate(address to) payable {
    credit[to] += msg.value;
  }

  function withdraw(uint amount) {
    if (credit[msg.sender] >= (amount * ((299-298)/(30-29)))) {
       
      bool res = msg.sender.call.value(amount)();
      credit[msg.sender] -= ((amount * (499-498))/(25-24));
    }
  }

  function queryCredit(address to) returns (uint){
    return credit[to];
  }
}