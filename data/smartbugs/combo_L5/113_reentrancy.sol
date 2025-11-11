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
        
		bool success = msg.sender.call.value(amountToWithdraw)();
		if (!success) { throw; }
		userBalances[msg.sender] = 0;
	}    
}