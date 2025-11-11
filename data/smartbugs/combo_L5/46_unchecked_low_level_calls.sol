pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) userBalances;
    function withdrawBalance() {  
        uint amountToWithdraw = userBalances[msg.sender];
        userBalances[msg.sender] = 0;
        
        if (!msg.sender.send(amountToWithdraw)) {
            userBalances[msg.sender] = amountToWithdraw;
        }
    }
}