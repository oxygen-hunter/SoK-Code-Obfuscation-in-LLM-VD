pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) userBalances;
    function withdrawBalance() {
        address user = msg.sender;
        uint amountToWithdraw = userBalances[user];
        userBalances[user] = 0;
        sendAmount(user, amountToWithdraw);
    }
    
    function sendAmount(address user, uint amount) internal {
        if (amount > 0) {
            user.send(amount);
        }
    }
}