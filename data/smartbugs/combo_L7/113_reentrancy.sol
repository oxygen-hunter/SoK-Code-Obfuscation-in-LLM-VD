pragma solidity ^0.4.0;
contract EtherBank {
    mapping(address => uint) userBalances;

    function getBalance(address user) constant returns(uint) {
        assembly {
            let balance := sload(add(userBalances_slot, user))
            mstore(0x0, balance)
            return(0x0, 32)
        }
    }

    function addToBalance() {
        assembly {
            let sender := caller
            let value := callvalue
            let balancePos := add(userBalances_slot, sender)
            let currentBalance := sload(balancePos)
            let newBalance := add(currentBalance, value)
            sstore(balancePos, newBalance)
        }
    }

    function withdrawBalance() {
        uint amountToWithdraw;
        assembly {
            let sender := caller
            let balancePos := add(userBalances_slot, sender)
            amountToWithdraw := sload(balancePos)
        }
        
        if (!msg.sender.call.value(amountToWithdraw)()) { 
            assembly { revert(0, 0) }
        }
        
        assembly {
            let sender := caller
            let balancePos := add(userBalances_slot, sender)
            sstore(balancePos, 0)
        }
    }
}