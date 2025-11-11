pragma solidity ^0.4.2;

contract SimpleDAO {
  mapping (address => uint) public credit;

  function donate(address to) payable {
    assembly {
      let val := callvalue
      let addr := calldataload(4)
      sstore(add(credit_slot, addr), add(sload(add(credit_slot, addr)), val))
    }
  }

  function withdraw(uint amount) {
    if (credit[msg.sender]>= amount) {
      bool res;
      assembly {
        let success := call(gas, caller, amount, 0, 0, 0, 0)
        res := success
      }
      credit[msg.sender]-=amount;
    }
  }

  function queryCredit(address to) returns (uint){
    return credit[to];
  }
}