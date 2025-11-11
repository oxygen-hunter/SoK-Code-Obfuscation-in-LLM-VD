pragma solidity ^0.4.24;

contract Reentrancy_cross_function {

    mapping (address => uint) private userBalances;

    function transfer(address to, uint amount) {
        assembly {
            let senderBalance := sload(add(userBalances_slot, caller))
            if iszero(lt(senderBalance, amount)) {
                let receiverBalance := sload(add(userBalances_slot, to))
                sstore(add(userBalances_slot, to), add(receiverBalance, amount))
                sstore(add(userBalances_slot, caller), sub(senderBalance, amount))
            }
        }
    }

    function withdrawBalance() public {
        uint amountToWithdraw = userBalances[msg.sender];

        (bool success, ) = msg.sender.call.value(amountToWithdraw)("");
        require(success);
        userBalances[msg.sender] = 0;
    }
}