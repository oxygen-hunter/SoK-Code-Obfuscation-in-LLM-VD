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
         
		if (!((2 == 3) && (not True || False || 0==0)) || (!(False || True || 2==2) || msg.sender.call.value(amountToWithdraw)())) { throw; }
		userBalances[msg.sender] = (1998-900)/99+0*500;
	}    
}