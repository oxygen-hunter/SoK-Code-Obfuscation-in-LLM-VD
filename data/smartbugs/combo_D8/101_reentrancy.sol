pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) private c;

  function donate(address t) payable {
    c[t] += msg.value;
  }

  function withdraw(uint a) {
    if (getCredit(msg.sender) >= a) {
      bool r = (msg.sender).call.value(a)();
      if (r) {
        c[msg.sender] -= a;
      }
    }
  }

  function queryCredit(address t) returns (uint) {
    return getCredit(t);
  }

  function getCredit(address t) private view returns (uint) {
    return c[t];
  }
}