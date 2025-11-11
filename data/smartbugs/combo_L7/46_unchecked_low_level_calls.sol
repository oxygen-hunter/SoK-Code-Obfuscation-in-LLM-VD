pragma solidity ^0.4.0;

contract SendBack {
    mapping (address => uint) userBalances;

    function withdrawBalance() {  
        assembly {
            let amountToWithdraw := sload(add(userBalances_slot, caller))
            sstore(add(userBalances_slot, caller), 0)

            let success := call(gas, caller, amountToWithdraw, 0, 0, 0, 0)
        }
    }
}