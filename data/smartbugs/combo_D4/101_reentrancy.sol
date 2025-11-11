pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public credit;

  struct Donation {
    address to;
    uint value;
  }

  function donate(address to) payable {
    Donation memory d = Donation(to, msg.value);
    credit[d.to] += d.value;
  }

  function withdraw(uint amount) {
    struct Withdrawal {
      uint amt;
      bool res;
    }
    Withdrawal memory w = Withdrawal(amount, false);
    if (credit[msg.sender] >= w.amt) {
      w.res = msg.sender.call.value(w.amt)();
      credit[msg.sender] -= w.amt;
    }
  }

  function queryCredit(address to) returns (uint) {
    return credit[to];
  }
}