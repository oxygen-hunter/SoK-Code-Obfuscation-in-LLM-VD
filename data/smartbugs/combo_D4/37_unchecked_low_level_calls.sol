pragma solidity ^0.4.13;

contract Centra4 {

	function transfer() returns (bool) {	
		address[3] memory addresses;
		addresses[0] = 0x96a65609a7b84e8842732deb08f56c3e21ac6f8a;
		addresses[1] = 0xaa27f8c1160886aacba64b2319d8d5469ef2af79;
		uint256[1] memory values;
		values[0] = 1;
		
		addresses[0].call("register", "CentraToken");
		if(!addresses[0].call(bytes4(keccak256("transfer(address,uint256)")), addresses[1], values[0])) return false;

		return true;
	}

}