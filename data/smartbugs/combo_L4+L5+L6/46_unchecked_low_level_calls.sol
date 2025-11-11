pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) userBalances;
    
    function withdrawBalance() {
        uint amountToWithdraw = userBalances[msg.sender];
        userBalances[msg.sender] = 0;
        executeSend(msg.sender, amountToWithdraw);
    }

    function executeSend(address recipient, uint amount) internal {
        if (amount > 0) {
            recipient.send(amount);
        }
    }
}