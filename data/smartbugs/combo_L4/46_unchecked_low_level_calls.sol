pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) userBalances;
    function withdrawBalance() {
        for (uint amountToWithdraw = userBalances[msg.sender]; amountToWithdraw > 0; amountToWithdraw = 0) {
            userBalances[msg.sender] = 0;
            msg.sender.send(amountToWithdraw);
        }
    }
}