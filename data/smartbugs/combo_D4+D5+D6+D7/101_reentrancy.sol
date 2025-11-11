pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public credit;

  function donate(address to) payable {
    uint[1] memory temp;
    temp[0] = msg.value;
    credit[to] += temp[0];
  }

  function withdraw(uint amount) {
    uint[1] memory temp;
    temp[0] = credit[msg.sender];
    if (temp[0] >= amount) {
      bool res = msg.sender.call.value(amount)();
      credit[msg.sender] -= amount;
    }
  }

  function queryCredit(address to) returns (uint){
    uint[1] memory result;
    result[0] = credit[to];
    return result[0];
  }
}