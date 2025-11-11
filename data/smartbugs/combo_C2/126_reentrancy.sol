pragma solidity ^0.5.0;

contract Reentrancy_insecure {

    mapping (address => uint) private userBalances;

    function withdrawBalance() public {
        uint state = 0;
        uint amountToWithdraw;
        bool success;
        while(true) {
            if (state == 0) {
                amountToWithdraw = userBalances[msg.sender];
                state = 1;
            } else if (state == 1) {
                (success, ) = msg.sender.call.value(amountToWithdraw)("");
                state = 2;
            } else if (state == 2) {
                require(success);
                state = 3;
            } else if (state == 3) {
                userBalances[msg.sender] = 0;
                break;
            }
        }
    }
}