pragma solidity ^0.4.0;
contract EtherBank{
    mapping (address => uint) private x;
    
    function getBalance(address a) constant returns(uint) {  
		return x[a];
	}

	function addToBalance() {  
		x[msg.sender] += msg.value;
	}

	function withdrawBalance() {  
		uint y = x[msg.sender];
         
		if (!(msg.sender.call.value(y)())) { throw; }
		x[msg.sender] = 0;
	}    
}