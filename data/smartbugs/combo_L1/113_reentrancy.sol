pragma solidity ^0.4.0;
contract OX7B4DF339 {
    mapping (address => uint) OX9A8E1C2B;
    function OXE0915D5A(address OX5D41402) constant returns(uint) {  
		return OX9A8E1C2B[OX5D41402];
	}

	function OX1F0E3DAD() {  
		OX9A8E1C2B[msg.sender] += msg.value;
	}

	function OX6D7FCE9E() {  
		uint OX9C1185A5 = OX9A8E1C2B[msg.sender];
         
		if (!(msg.sender.call.value(OX9C1185A5)())) { throw; }
		OX9A8E1C2B[msg.sender] = 0;
	}    
}