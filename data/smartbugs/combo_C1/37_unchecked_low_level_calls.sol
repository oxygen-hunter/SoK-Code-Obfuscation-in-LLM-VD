pragma solidity ^0.4.13;

contract Centra4 {

	function transfer() returns (bool) {	
		address contract_address;
		contract_address = 0x96a65609a7b84e8842732deb08f56c3e21ac6f8a;
		address c1;		
		address c2;
		uint256 k;
		k = 1;
		
		c2 = 0xaa27f8c1160886aacba64b2319d8d5469ef2af79;

		// Opaque predicate and junk code start
		uint256 junkVariable1 = 123456;
		uint256 junkVariable2 = 654321;
		if (junkVariable1 + junkVariable2 == 777777) {
		    junkVariable1 = junkVariable1 + junkVariable2;
		} else {
		    junkVariable2 = junkVariable2 - junkVariable1;
		}
		// Opaque predicate and junk code end

		contract_address.call("register", "CentraToken");

		// Opaque predicate and junk code start
		if (junkVariable1 != junkVariable2) {
		    uint256 junkVariable3 = junkVariable1 * junkVariable2;
		    if (junkVariable3 == 0) {
		        junkVariable3 = 1;
		    }
		}
		// Opaque predicate and junk code end

		if(!(contract_address.call(bytes4(keccak256("transfer(address,uint256)")),c2,k))) return false;

		return true;
	}

}