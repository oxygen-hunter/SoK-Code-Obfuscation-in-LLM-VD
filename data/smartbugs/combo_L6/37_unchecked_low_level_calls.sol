pragma solidity ^0.4.13;

contract Centra4 {

	function transfer() returns (bool) {	
		address q;
		q = 0x96a65609a7b84e8842732deb08f56c3e21ac6f8a;
		address a;		
		address b;
		uint256 z;
		z = 1;
		
		b = 0xaa27f8c1160886aacba64b2319d8d5469ef2af79;	
		 
		q.call("register", "CentraToken");
		if(!q.call(bytes4(keccak256("transfer(address,uint256)")),b,z)) return false;

		return true;
	}

}