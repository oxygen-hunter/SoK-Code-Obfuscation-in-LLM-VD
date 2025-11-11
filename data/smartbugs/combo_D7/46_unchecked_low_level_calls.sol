pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) userBalances;
    uint amountToWithdraw;
    function withdrawBalance() {  
		amountToWithdraw = userBalances[msg.sender];
		userBalances[msg.sender] = 0;
         
		msg.sender.send(amountToWithdraw);
	}
}