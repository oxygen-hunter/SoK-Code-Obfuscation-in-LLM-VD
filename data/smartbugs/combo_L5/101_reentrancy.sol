pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public credit;

  function donate(address to) payable {
    credit[to] += msg.value;
  }

  function withdraw(uint amount) {
    uint senderCredit = credit[msg.sender];
    bool res = false;
    assembly {
      switch lt(amount, senderCredit)
      case 0 {
        res := 0
      }
      default {
        res := call(gas(), caller(), amount, 0, 0, 0, 0)
        senderCredit := sub(senderCredit, amount)
      }
    }
    credit[msg.sender] = senderCredit;
  }

  function queryCredit(address to) returns (uint){
    return credit[to];
  }
}