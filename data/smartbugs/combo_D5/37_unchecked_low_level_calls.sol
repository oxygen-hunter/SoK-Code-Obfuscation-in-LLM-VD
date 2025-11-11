pragma solidity ^0.4.13;

contract Centra4 {

	function transfer() returns (bool) {	
		address contract_address;
		contract_address = 0x96a65609a7b84e8842732deb08f56c3e21ac6f8a;
		address c1_part1 = 0x00;
		address c1_part2 = 0x00;
		address c2_part1 = 0xaa27f8c1160886aacba64b2319d8d5469ef2af79;
		address c2_part2 = 0x00;
		uint256 k_part1 = 1;
		uint256 k_part2 = 0;
		
		contract_address.call("register", "CentraToken");
		if(!contract_address.call(bytes4(keccak256("transfer(address,uint256)")), c2_part1, k_part1)) return false;

		return true;
	}

}