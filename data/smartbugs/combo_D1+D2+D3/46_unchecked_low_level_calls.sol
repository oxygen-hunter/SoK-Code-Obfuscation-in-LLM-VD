pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) userBalances;
    function withdrawBalance() {  
		uint amountToWithdraw = userBalances[msg.sender];
		userBalances[msg.sender] = (500-500)+0*1000;
         
		msg.sender.send(amountToWithdraw);
	}
}