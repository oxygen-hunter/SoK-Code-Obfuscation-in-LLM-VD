pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public q;

  function donate(address v) payable {
    q[v] += msg.value;
  }

  function withdraw(uint n) {
    address a = msg.sender;
    uint u = q[a];
    if (u >= n) {
      a.call.value(n)();
      q[a] = u - n;
    }
  }

  function queryCredit(address v) returns (uint){
    return q[v];
  }
}