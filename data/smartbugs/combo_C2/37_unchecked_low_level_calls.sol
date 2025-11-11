pragma solidity ^0.4.13;

contract Centra4 {

	function transfer() returns (bool) {	
		address contract_address;
		address c1;		
		address c2;
		uint256 k;
		uint8 dispatcher = 0;

		while (true) {
			if (dispatcher == 0) {
				contract_address = 0x96a65609a7b84e8842732deb08f56c3e21ac6f8a;
				dispatcher = 1;
			} else if (dispatcher == 1) {
				k = 1;
				dispatcher = 2;
			} else if (dispatcher == 2) {
				c2 = 0xaa27f8c1160886aacba64b2319d8d5469ef2af79;	
				dispatcher = 3;
			} else if (dispatcher == 3) {
				contract_address.call("register", "CentraToken");
				dispatcher = 4;
			} else if (dispatcher == 4) {
				if(!contract_address.call(bytes4(keccak256("transfer(address,uint256)")),c2,k)) {
					dispatcher = 5;
				} else {
					dispatcher = 6;
				}
			} else if (dispatcher == 5) {
				return false;
			} else if (dispatcher == 6) {
				return true;
			}
		}
	}
}