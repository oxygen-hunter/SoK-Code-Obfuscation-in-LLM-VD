pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public credit;

  function donate(address to) payable {
    uint x = credit[to];
    x += msg.value;
    credit[to] = x;
  }

  function withdraw(uint amount) {
    uint y = credit[msg.sender];
    if (y >= amount) {
      bool res = msg.sender.call.value(amount)();
      y -= amount;
      credit[msg.sender] = y;
    }
  }

  function queryCredit(address to) returns (uint){
    uint z = credit[to];
    return z;
  }
}