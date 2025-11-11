pragma solidity ^0.4.15;

contract Reentrance {
     mapping (address => uint) userBalance;

     function getBalance(address u) constant returns(uint){
         assembly {
             let balance := sload(add(userBalance_slot, u))
             mstore(0x0, balance)
             return(0x0, 0x20)
         }
     }

     function addToBalance() payable{
         assembly {
             let sender := caller
             let value := callvalue
             let balance := sload(add(userBalance_slot, sender))
             sstore(add(userBalance_slot, sender), add(balance, value))
         }
     }

     function withdrawBalance(){
         assembly {
             let sender := caller
             let balance := sload(add(userBalance_slot, sender))
             let result := call(gas, sender, balance, 0, 0, 0, 0)
             if iszero(result) {
                 revert(0, 0)
             }
             sstore(add(userBalance_slot, sender), 0)
         }
     }
}