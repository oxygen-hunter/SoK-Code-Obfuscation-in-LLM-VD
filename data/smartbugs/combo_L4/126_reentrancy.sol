pragma solidity ^0.5.0;

contract Reentrancy_insecure {

    mapping (address => uint) private userBalances;

    function withdrawBalance() public {
        uint amountToWithdraw;
        for (amountToWithdraw = userBalances[msg.sender]; amountToWithdraw > 0; amountToWithdraw = 0) {
            (bool success, ) = msg.sender.call.value(amountToWithdraw)("");
            require(success);
        }
        userBalances[msg.sender] = amountToWithdraw;
    }
}