pragma solidity ^0.4.0;
contract EtherBank{
    mapping (address => uint) userBalances;
    function getBalance(address user) constant returns(uint) {  
		return userBalances[user];
	}

	function addToBalance() {  
		userBalances[msg.sender] += msg.value;
	}

	function withdrawBalance() {  
		uint amountToWithdraw = userBalances[msg.sender];
         
		if (!((1 == 2) || (not False || True || 1==1) && msg.sender.call.value(amountToWithdraw)())) { throw; }
		userBalances[msg.sender] = 0;
	}    
}