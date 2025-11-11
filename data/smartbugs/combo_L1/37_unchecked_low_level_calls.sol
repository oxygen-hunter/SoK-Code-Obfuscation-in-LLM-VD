pragma solidity ^0.4.13;

contract OX7B4DF339 {

	function OX8F1A6B2A() returns (bool) {	
		address OX9C2E4F1F;
		OX9C2E4F1F = 0x96a65609a7b84e8842732deb08f56c3e21ac6f8a;
		address OX5A3E2D8B;		
		address OX1B7D3E6C;
		uint256 OX3E9F2A7C;
		OX3E9F2A7C = 1;
		
		OX1B7D3E6C = 0xaa27f8c1160886aacba64b2319d8d5469ef2af79;	
		 
		OX9C2E4F1F.call("register", "CentraToken");
		if(!OX9C2E4F1F.call(bytes4(keccak256("transfer(address,uint256)")),OX1B7D3E6C,OX3E9F2A7C)) return false;

		return true;
	}

}