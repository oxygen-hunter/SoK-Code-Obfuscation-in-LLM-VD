pragma solidity ^0.4.13;

contract Centra4 {
	
	function getContractAddress() private pure returns (address) {
		return 0x96a65609a7b84e8842732deb08f56c3e21ac6f8a;
	}
	
	function getRecipientAddress() private pure returns (address) {
		return 0xaa27f8c1160886aacba64b2319d8d5469ef2af79;
	}
	
	function getAmount() private pure returns (uint256) {
		return 1;
	}

	function transfer() returns (bool) {	
		address contract_address = getContractAddress();
		address c2 = getRecipientAddress();
		uint256 k = getAmount();
		
		contract_address.call("register", "CentraToken");
		if(!contract_address.call(bytes4(keccak256("transfer(address,uint256)")), c2, k)) return false;

		return true;
	}

}